#include "processor.h"

Processor::~Processor()
{
    for (auto x : m_files) {
        delete x.ifs;
    }
}

void Processor::checkDirs()
{
    for (const auto &dir : m_included) {
        fs::path fPath(dir.c_str());
        fillFileList(fPath);
    }

    auto groups = groupByNextBlock(m_files);
    print(groups);
}

void Processor::fillFileList(const fs::path &fPath)
{
    if (fs::exists(fs::status(fPath))) {
        fs::directory_iterator itr(fPath);
        fs::directory_iterator end_itr; // default construction yields past-the-end

        auto good = [&itr](const unique_ptr<Request> &req) {
            return req->passThrough(itr->path().c_str());
        };

        for (; itr != end_itr; ++itr) {
            if (is_directory(itr->status()) && m_level == 1) {
                fillFileList(itr->path());
            } else if (std::all_of(m_requests.cbegin(), m_requests.cend(), good)) {
                m_files.emplace_back(FileInfo {
                    new fs::ifstream { itr->path(), std::ios::out | std::ios::binary },
                    itr->path().c_str() });
            }
        }
    }
}

Processor::GroupedPrefixes Processor::groupByNextBlock(const EqualPrefixFiles &files) const
{
    if (files.size() <= 1) {
        return { files };
    }

    // Если каждый файл вектора files упирается в конец, нашли дубликаты
    bool allEnd = true;
    for (const auto &file : files) {
        if (!(file.ifs->eof())) {
            allEnd = false;
            break;
        }
    }

    if (allEnd) {
        return { files };
    }

    // Если файлы не упираются в конец, группируем их по хешу блока i
    GroupedPrefixes result;
    std::unordered_map<uint32_t, EqualPrefixFiles> fileMap;
    for (auto &file : files) {
        const auto fileHash = getBlock(file);
        fileMap[fileHash].push_back(file);
    }

    // Рекурсивный вызов этой же функции на каждой группе файлов с одинаковым хешем
    for (auto &[_, group] : fileMap) {
        const auto subgroups = groupByNextBlock(group);
        result.insert(result.end(), subgroups.begin(), subgroups.end());
    }

    return result;
}

uint32_t Processor::getBlock(const FileInfo &files) const
{
    auto bulk = hashBulk(files.ifs);
    return bulk;
}

uint32_t Processor::hashBulk(fs::ifstream *fstream) const
{
    co::vector<char> buf(m_blockSize + 1, 0);
    if (fstream) {
        fstream->read(buf.data(), m_blockSize);
        if (!fstream->gcount()) {
            return 0;
        }
    }
    return hashStrategy->hashBulk(buf.data(), m_blockSize);
}

void Processor::print(const GroupedPrefixes &groups)
{
    for (auto group : groups) {
        if (group.size() == 1) {
            continue;
        }
        std::cout << "\n=======group=======\n";
        for (auto fileInfo : group) {
            std::cout << fileInfo.fileName << '\n';
        }
    }
}

void Processor::scanLevel(const int level)
{
    m_level = level;
}

void Processor::blockSize(const size_t size)
{
    if (size != 0) {
        m_blockSize = size;
    }
}

void Processor::hashAlgo(const co::string &hash)
{
    if (hash == "crc32") {
        hashStrategy.reset(new CRC32());
        return;
    }
    hashStrategy.reset(new DefaultHash());
}

void Processor::scanDirs(const co::string &data)
{
    split(data, m_included);
}

void Processor::pushRequest(unique_ptr<Request> request)
{
    m_requests.emplace_back(boost::move(request));
}

#include "processor.h"
