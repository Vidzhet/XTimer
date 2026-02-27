#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <type_traits>
#include <stdexcept>
#include <cstdint>

namespace vidzhet {
    enum Mode { Read, Write };

    class ConfigCreator {

    private:
        struct HeaderInfo {
            std::string name;
            uint64_t data_size;
            uint64_t data_offset;
        };

        std::fstream file_;
        Mode mode_;
        std::vector<HeaderInfo> headers_;
        static constexpr char MAGIC[3] = { 'M', 'G', 'E' };

    public:
        ConfigCreator(std::string filename, Mode mode)
            : mode_(mode)
        {
            if (filename.size() < 4 || filename.compare(filename.size() - 4, 4, ".mge") != 0)
            {
                filename += ".mge";
            }
            if (mode == Mode::Write) {
                file_.open(filename, std::ios::binary | std::ios::out | std::ios::trunc);
                if (!file_) { throw std::runtime_error("Failed to open file"); }

                file_.write(MAGIC, sizeof(MAGIC));
            }
            else {
                file_.open(filename, std::ios::binary | std::ios::in);
                if (!file_) { throw std::runtime_error("Failed to open file"); }

                char magic_check[sizeof(MAGIC)];
                file_.read(magic_check, sizeof(MAGIC));
                if (!file_ || std::memcmp(magic_check, MAGIC, sizeof(MAGIC)) != 0) {
                    throw std::runtime_error("File is not valid");
                }

                parse_headers();
            }
        }

        ~ConfigCreator() {
            close();
        }

        // close the file. also called in destructor
        void close() {
            if (file_.is_open()) file_.close();
        }

    private:
        void parse_headers() {
            while (true) {
                uint64_t name_size;

                if (!file_.read(reinterpret_cast<char*>(&name_size), sizeof(name_size))) {
                    if (file_.eof()) break;
                    throw std::runtime_error("Corrupted header (name_size)");
                }

                std::string name(name_size, '\0');
                if (!file_.read(&name[0], name_size))
                    throw std::runtime_error("Failed to read header name");

                uint64_t data_size;
                if (!file_.read(reinterpret_cast<char*>(&data_size), sizeof(data_size)))
                    throw std::runtime_error("Failed to read header size");

                uint64_t data_offset = static_cast<uint64_t>(file_.tellg());
                headers_.push_back({ name, data_size, data_offset });

                file_.seekg(data_size, std::ios::cur);
            }
            file_.clear();
        }

        HeaderInfo* find_header(const std::string& name) {
            for (auto& h : headers_) {
                if (h.name == name) return &h;
            }
            return nullptr;
        }

    public:

        // ---------------- Header Iterator ----------------
        template<typename T>
        class HeaderIterator {
            std::fstream* file_;
            uint64_t data_start_;
            uint64_t data_size_;
            uint64_t offset_;
            bool write_mode_;
            uint64_t size_pos_;

        public:
            HeaderIterator(std::fstream* f, uint64_t a, uint64_t b, Mode mode)
                : file_(f), offset_(0)
            {
                if (mode == Mode::Read) {
                    data_start_ = a;
                    data_size_ = b;
                    write_mode_ = false;
                    size_pos_ = 0;
                }
                else {
                    size_pos_ = a;
                    data_start_ = b;
                    data_size_ = 0;
                    write_mode_ = true;
                }
            }

            ~HeaderIterator() { if (write_mode_) finalize(); }

            HeaderIterator(const HeaderIterator&) = delete;
            HeaderIterator& operator=(const HeaderIterator&) = delete;
            HeaderIterator(HeaderIterator&& other) noexcept
                : file_(other.file_), data_start_(other.data_start_), data_size_(other.data_size_),
                offset_(other.offset_), write_mode_(other.write_mode_), size_pos_(other.size_pos_) {
                other.write_mode_ = false;
            }

            // ---------------- WRITE ----------------

            void write(const T& value) {
                if constexpr (std::is_trivially_copyable_v<T>) {
                    file_->write(reinterpret_cast<const char*>(&value), sizeof(T));
                    offset_ += sizeof(T);
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    uint64_t len = value.size();
                    file_->write(reinterpret_cast<const char*>(&len), sizeof(len));
                    file_->write(value.data(), len);
                    offset_ += sizeof(len) + len;
                }
                else {
                    static_assert(sizeof(T) == 0, "Unsupported type for HeaderIterator::write");
                }
            }

            // close the header. always call it when you done writing in header
            void finalize() {
                if (!write_mode_) return;
                uint64_t end_pos = static_cast<uint64_t>(file_->tellp());
                data_size_ = end_pos - data_start_;
                file_->seekp(size_pos_);
                file_->write(reinterpret_cast<char*>(&data_size_), sizeof(data_size_));
                file_->seekp(end_pos);
                write_mode_ = false;
            }

            // ---------------- READ ----------------

            // can read the next item in header?
            bool next() const {
                return offset_ < data_size_;
            }


            // reads an item in the header
            auto read()
            { // i hope it returns something, compiler dies if i try to return template type 
                if (!next()) {
                    throw std::runtime_error("Out of bounds");
                }

                std::streampos old_pos = file_->tellg();
                if constexpr (std::is_trivially_copyable_v<T>) {
                    T value;
                    file_->seekg(data_start_ + offset_);
                    file_->read(reinterpret_cast<char*>(&value), sizeof(T));
                    offset_ += sizeof(T);
                    file_->seekg(old_pos);
                    return value;
                }
                else if constexpr (std::is_same_v<T, std::string>) {
                    uint64_t len;
                    file_->seekg(data_start_ + offset_);
                    file_->read(reinterpret_cast<char*>(&len), sizeof(len));
                    std::string s(len, '\0');
                    file_->read(&s[0], len);
                    offset_ += sizeof(len) + len;
                    file_->seekg(old_pos);
                    return s;
                }
                else {
                    static_assert(sizeof(T) == 0, "Unsupported type for HeaderIterator::read");
                }
            }
        };

        template<typename T>
        T read_static(const std::string& name) {
            //static_assert(std::is_trivially_copyable_v<T>, "Only trivially copyable types supported");

            auto h = header<T>(name);
            return h.read();
        }

        // ---------------- Add Header ----------------
        template<typename T>
        HeaderIterator<T> addheader(const std::string& name) {
            if (mode_ != Mode::Write) throw std::runtime_error("Not in write mode");

            uint64_t name_size = name.size();
            file_.write(reinterpret_cast<char*>(&name_size), sizeof(name_size));
            file_.write(name.data(), name_size);
            if (!file_) throw std::runtime_error("Failed to write header name");

            uint64_t size_pos = static_cast<uint64_t>(file_.tellp());
            uint64_t zero = 0;
            file_.write(reinterpret_cast<char*>(&zero), sizeof(zero));

            uint64_t data_start = static_cast<uint64_t>(file_.tellp());
            return HeaderIterator<T>(&file_, size_pos, data_start, Mode::Write); // keep constant write mode here?
        }

        template<typename T> // get the header iterator
        HeaderIterator<T> header(const std::string& name) {
            auto* h = find_header(name);
            if (!h) throw std::runtime_error("Header not found");

            if constexpr (std::is_trivially_copyable_v<T>) {
                if (h->data_size % sizeof(T) != 0)
                    throw std::runtime_error("Corrupted header size");
            }

            return HeaderIterator<T>(&file_, h->data_offset, h->data_size, mode_);
        }

        template<typename T>
        void additem(const std::string& name, const T& value) {
            auto h = addheader<T>(name);
            h.write(value);
        }

        void additem(const std::string& name, const char* value) {
            additem<std::string>(name, value);
        }
        void additem(const std::string& name, std::string& value) {
            additem<std::string>(name, value);
        }
    };
}

using vidzhet::Write;
using vidzhet::Read;
