#include "processor.h"

Processor::Processor() { }

Processor::~Processor()
{
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

void Processor::hashAlgo(const string &hash)
{
    if (hash == "crc32") {
        hashStrategy.reset(new CRC32());
        return;
    }
    hashStrategy.reset(new DefaultHash());
}

void Processor::scanDirs(const string &data)
{
    split(data, m_included);
}

void Processor::pushRequest(unique_ptr<Request> request)
{
    m_requests.emplace_back(boost::move(request));
}

void Processor::print()
{
    if (filesAtCurrentPath.empty()) {
        return;
    }

    std::sort(filesAtCurrentPath.begin(), filesAtCurrentPath.end(),
          [](const FileInfo &l, const FileInfo &r) { return l.id > r.id; });

    uuid *id = &filesAtCurrentPath.at(0).id;

    for (auto &fileInfo : filesAtCurrentPath) {
        if (fileInfo.id.is_nil()) {
            break;
        }

        if (*id == fileInfo.id) {
            std::cout << fileInfo.path << '\n';
        } else {
            std::cout << '\n';
            std::cout << fileInfo.path << '\n';
            id = &fileInfo.id;
        }
    }
    std::cout << std::endl;
}

void Processor::checkFile()
{
    for (const auto &dir : m_included) {
        fs::path fPath(dir.c_str());
        fillFileList(fPath);
    }

    if (filesAtCurrentPath.size() > 1) {
        fileInfoIterator it(filesAtCurrentPath.begin());
        fileInfoIterator it1(boost::next(filesAtCurrentPath.begin()));

        while (it1 != boost::end(filesAtCurrentPath)) {
            compareFiles(it, it1);
            ++it;
            it1 = boost::next(it);
        }
        print();
    }
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
            } else if (is_regular_file(itr->status())
                   && std::all_of(m_requests.cbegin(), m_requests.cend(), good)) {
                filesAtCurrentPath.emplace_back(FileInfo { itr->path() });
            }
        }
    }
}

void Processor::compareFiles(fileInfoIterator &it, fileInfoIterator &it1)
{
    fs::ifstream ifs { it->path, std::ios_base::openmode::_S_out | std::ios_base::openmode::_S_bin };

    for (; it1 != boost::end(filesAtCurrentPath); ++it1) {
        fs::ifstream ifs1 { it1->path, std::ios_base::openmode::_S_out
                               | std::ios_base::openmode::_S_bin };
        int i = 0;
        while (true) {
            auto block = getBlock(it, ifs, i);
            auto block1 = getBlock(it1, ifs1, i);

            if (block == block1 && !(ifs.eof() || ifs1.eof())) {
                ++i;
                continue;
            }

            if (block != block1) {
                break;
            }

            // okay good, let's mark our match with the unique key,
            // so the same items has the same id and would not be lost at container.
            if (it->id.is_nil()) {
                it->id = gen();
            }

            it1->id = it->id;
            break;
        }
        ifs1.close();
    }

    ifs.close();
}

uint32_t Processor::getBlock(fileInfoIterator &it, fs::ifstream &ifs, const size_t block)
{
    if (it->blocks.size() > block) {
        ifs.seekg((block + 1) * m_blockSize);
        return it->blocks.at(block);
    }
    auto bulk = hashBulk(ifs);
    if (bulk) {
        it->blocks.push_back(bulk);
    }
    return bulk;
}

uint32_t Processor::hashBulk(fs::ifstream &fstream)
{
    co::vector<char> buf(m_blockSize + 1, 0);
    fstream.read(buf.data(), m_blockSize);

    if (!fstream.gcount()) {
        return 0;
    }

    auto s = hashStrategy->hashBulk(buf.data(), m_blockSize);
    return s;
}
