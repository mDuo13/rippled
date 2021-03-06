//
// Copyright (c) 2013-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "nodejs_parser.hpp"
#include "message_fuzz.hpp"
#include <beast/http.hpp>
#include <beast/streambuf.hpp>
#include <beast/buffers_debug.hpp>
#include <beast/detail/unit_test/suite.hpp>
#include <chrono>
#include <iostream>
#include <vector>

namespace beast {
namespace http {

class parser_bench_test : public beast::detail::unit_test::suite
{
public:
    static std::size_t constexpr N = 2000;

    using corpus = std::vector<streambuf>;

    corpus creq_;
    corpus cres_;
    std::size_t size_ = 0;

    parser_bench_test()
    {
        creq_ = build_corpus(N/2, std::true_type{});
        cres_ = build_corpus(N/2, std::false_type{});
    }

    corpus
    build_corpus(std::size_t n, std::true_type)
    {
        corpus v;
        v.resize(N);
        message_fuzz mg;
        for(std::size_t i = 0; i < n; ++i)
        {
            mg.request(v[i]);
            size_ += v[i].size();
        }
        return v;
    }

    corpus
    build_corpus(std::size_t n, std::false_type)
    {
        corpus v;
        v.resize(N);
        message_fuzz mg;
        for(std::size_t i = 0; i < n; ++i)
        {
            mg.response(v[i]);
            size_ += v[i].size();
        }
        return v;
    }

    template<class Parser>
    void
    testParser(std::size_t repeat, corpus const& v)
    {
        while(repeat--)
            for(auto const& sb : v)
            {
                Parser p;
                error_code ec;
                p.write(sb.data(), ec);
                if(! expect(! ec, ec.message()))
                    log << debug::buffers_to_string(
                        sb.data()) << std::endl;
            }
    }

    template<class Function>
    void
    timedTest(std::size_t repeat, std::string const& name, Function&& f)
    {
        using namespace std::chrono;
        using clock_type = std::chrono::high_resolution_clock;
        log << name;
        for(std::size_t trial = 1; trial <= repeat; ++trial)
        {
            auto const t0 = clock_type::now();
            f();
            auto const elapsed = clock_type::now() - t0;
            log <<
                "Trial " << trial << ": " <<
                duration_cast<milliseconds>(elapsed).count() << " ms";
        }
    }

    template<bool isRequest>
    struct null_parser : basic_parser<isRequest, null_parser<isRequest>>
    {
    };

    void
    testSpeed()
    {
        static std::size_t constexpr Trials = 3;
        static std::size_t constexpr Repeat = 50;

        log << "sizeof(request parser)  == " <<
            sizeof(basic_parser<true, null_parser<true>>);

        log << "sizeof(response parser) == " <<
            sizeof(basic_parser<false, null_parser<true>>);

        testcase << "Parser speed test, " <<
            ((Repeat * size_ + 512) / 1024) << "KB in " <<
                (Repeat * (creq_.size() + cres_.size())) << " messages";

        timedTest(Trials, "nodejs_parser",
            [&]
            {
                testParser<nodejs_parser<
                    true, streambuf_body, http_headers>>(
                        Repeat, creq_);
                testParser<nodejs_parser<
                    false, streambuf_body, http_headers>>(
                        Repeat, cres_);
            });
        timedTest(Trials, "http::basic_parser",
            [&]
            {
                testParser<parser<
                    true, streambuf_body, http_headers>>(
                        Repeat, creq_);
                testParser<parser<
                    false, streambuf_body, http_headers>>(
                        Repeat, cres_);
            });
        pass();
    }

    void run() override
    {
        pass();
        testSpeed();
    }
};

BEAST_DEFINE_TESTSUITE(parser_bench,http,beast);

} // http
} // beast

