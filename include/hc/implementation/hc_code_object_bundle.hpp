//===----------------------------------------------------------------------===//
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#pragma once

#include <hsa/hsa.h>

#include <algorithm>
#include <cstdint>
#include <istream>
#include <iterator>
#include <string>
#include <vector>

namespace hc
{
    namespace detail
    {
        struct Bundled_code {
            union {
                struct {
                    std::uint64_t offset;
                    std::uint64_t bundle_sz;
                    std::uint64_t triple_sz;
                };
                char cbuf[
                    sizeof(offset) + sizeof(bundle_sz) + sizeof(triple_sz)];
            };
            std::string triple;
            std::vector<char> blob;
        };

        class Bundled_code_header_base {
            friend class Bundled_code_header;

            static
            constexpr
            const char* magic_string_()
            {
                return "__CLANG_OFFLOAD_BUNDLE__";
            }
            static
            constexpr
            std::size_t strlen_(
                const char* ptr, std::size_t n = 0u) noexcept
            {
                return ptr ? (*ptr ? strlen_(ptr + 1, n + 1) : n) : n;
            }
        };

        class Bundled_code_header : private Bundled_code_header_base {
            using Bundled_code_header_base::strlen_;

            friend
            inline
            bool valid(const Bundled_code_header& x)
            {
                return std::equal(
                    x.bundler_magic_string,
                    x.bundler_magic_string + x.strlen_(x.magic_string_()),
                    x.magic_string_());
            }

            friend
            inline
            const std::vector<Bundled_code>& bundles(
                const Bundled_code_header& x)
            {
                return x.bundles;
            }

            template<typename RandomAccessIterator>
            friend
            inline
            bool read(
                RandomAccessIterator f,
                RandomAccessIterator,
                Bundled_code_header& x)
            {
                std::copy_n(f, sizeof(x.cbuf), x.cbuf);

                if (!valid(x)) return false;

                x.bundles.resize(x.bundle_cnt);

                auto it = f + sizeof(x.cbuf);
                for (auto&& y : x.bundles) {
                    std::copy_n(it, sizeof(y.cbuf), y.cbuf);
                    it += sizeof(y.cbuf);

                    y.triple.assign(it, it + y.triple_sz);

                    std::copy_n(
                        f + y.offset, y.bundle_sz, std::back_inserter(y.blob));

                    it += y.triple_sz;
                }

                return true;
            }

            friend
            inline
            bool read(const std::vector<char>& blob, Bundled_code_header& x)
            {
                return read(blob.cbegin(), blob.cend(), x);
            }

            friend
            inline
            bool read(std::istream& is, Bundled_code_header& x)
            {
                return read(std::vector<char>{
                    std::istreambuf_iterator<char>{is},
                    std::istreambuf_iterator<char>{}},
                    x);
            }

            union {
                struct {
                    char bundler_magic_string[strlen_(magic_string_())];
                    std::uint64_t bundle_cnt;
                };
                char cbuf[sizeof(bundler_magic_string) + sizeof(bundle_cnt)];
            };
            std::vector<Bundled_code> bundles;
        public:
            Bundled_code_header() = default;
            Bundled_code_header(const Bundled_code_header&) = default;
            Bundled_code_header(Bundled_code_header&&) = default;

            template<typename RandomAccessIterator>
            Bundled_code_header(RandomAccessIterator f, RandomAccessIterator l)
                : Bundled_code_header{}
            {
                read(f, l, *this);
            }

            explicit
            Bundled_code_header(const std::vector<char>& blob)
                : Bundled_code_header{blob.cbegin(), blob.cend()}
            {}
        };

        inline
        std::string transmogrify_triple(const std::string& triple)
        {
            static constexpr const char old_prefix[]{"hcc-amdgcn--amdhsa-gfx"};
            static constexpr const char new_prefix[]{
                "hcc-amdgcn-amd-amdhsa--gfx"};

            if (triple.find(old_prefix) == 0) {
                return new_prefix + triple.substr(sizeof(old_prefix) - 1);
            }

            return (triple.find(new_prefix) == 0) ? triple : "";
        }

        inline
        std::string isa_name(std::string triple)
        {
            static constexpr const char offload_prefix[]{"hcc-"};

            triple = transmogrify_triple(triple);
            if (triple.empty()) return {};

            triple.erase(0, sizeof(offload_prefix) - 1);

            static hsa_isa_t r{};
            static const bool is_old_rocr{
                hsa_isa_from_name(triple.c_str(), &r) != HSA_STATUS_SUCCESS};

            if (!is_old_rocr) return triple;

            auto tmp{triple.substr(triple.rfind('x') + 1)};
            triple.replace(0, std::string::npos, "AMD:AMDGPU");

            for (auto&& x : tmp) {
                triple.push_back(':');
                triple.push_back(x);
            }

            return triple;
        }

        inline
        hsa_isa_t triple_to_hsa_isa(std::string triple)
        {
            const auto isa = isa_name(std::move(triple));

            if (isa.empty()) return {};

            hsa_isa_t r{};
            const auto s = hsa_isa_from_name(isa.c_str(), &r);

            return (s == HSA_STATUS_SUCCESS) ? r : hsa_isa_t{};
        }
    } // Namespace hc::detail.
} // Namespace hc.
