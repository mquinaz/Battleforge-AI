// no #pragma once because we want this code to be duplicated

#include <string>
#include <utility>
#include <vector>
#ifdef C_LIKE_API
#include "TypesCLike.hpp"
#define bot_api capi
#else
#include "Types.hpp"
#define bot_api api
#endif

#include "fields_alloc.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>
#include <string>

#include "json.hpp"

namespace bot_api {
class IBotImp
{
public:
	explicit IBotImp(std::string name) : Name{std::move( name )} { }
	virtual ~IBotImp() = default;
	virtual std::vector<bot_api::Deck> DecksForMap(const bot_api::MapInfo& mapInfo) = 0;
	virtual void PrepareForBattle(const bot_api::MapInfo& mapInfo, const bot_api::Deck& deck) = 0;
	virtual void MatchStart(const bot_api::GameStartState& state) = 0;
	virtual std::vector<bot_api::Command> Tick(const bot_api::GameState& state) = 0;

	const std::string Name;
};

namespace wrapper_imp {
    void run(IBotImp& bot, unsigned short port);
}

static void run(IBotImp& bot, unsigned short port = 6370)
{
    wrapper_imp::run(bot, port);
}

namespace wrapper_imp {

    using json = nlohmann::json;

    namespace beast = boost::beast;         // from <boost/beast.hpp>
    namespace http = beast::http;           // from <boost/beast/http.hpp>
    namespace net = boost::asio;            // from <boost/asio.hpp>
    using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

    class http_worker
    {
    public:
        http_worker(http_worker const&) = delete;
        http_worker& operator=(http_worker const&) = delete;

        http_worker(tcp::acceptor& acceptor, IBotImp& bot) :
            acceptor_(acceptor), bot(bot)
        {
        }

        void start()
        {
            accept();
            check_deadline();
        }

    private:
        using alloc_t = fields_alloc<char>;
        //using request_body_t = http::basic_dynamic_body<beast::flat_static_buffer<1024 * 1024>>;
        using request_body_t = http::string_body;

        // The acceptor used to listen for incoming connections.
        tcp::acceptor& acceptor_;

        // The socket for the currently connected client.
        tcp::socket socket_{ acceptor_.get_executor() };

        // The buffer for performing reads
        beast::flat_static_buffer<8192> buffer_;

        // The allocator used for the fields in the request and reply.
        alloc_t alloc_{ 8192 };

        // The parser for reading the requests
        boost::optional<http::request_parser<request_body_t, alloc_t>> parser_;

        // The timer putting a time limit on requests.
        net::steady_timer request_deadline_{
            acceptor_.get_executor(), (std::chrono::steady_clock::time_point::max)() };

        // The string-based response message.
        boost::optional<http::response<http::string_body, http::basic_fields<alloc_t>>> string_response_;

        // The string-based response serializer.
        boost::optional<http::response_serializer<http::string_body, http::basic_fields<alloc_t>>> string_serializer_;

        IBotImp& bot;

        void accept()
        {
            // Clean up any previous connection.
            beast::error_code ec;
            socket_.close(ec);
            buffer_.consume(buffer_.size());

            acceptor_.async_accept(
                socket_,
                [this](beast::error_code ec)
                {
                    if (ec)
                    {
                        accept();
                    }
                    else
                    {
                        // Request must be fully processed within 60 seconds.
                        request_deadline_.expires_after(
                            std::chrono::seconds(60));

                        read_request();
                    }
                });
        }

        void read_request()
        {
            // On each read the parser needs to be destroyed and
            // recreated. We store it in a boost::optional to
            // achieve that.
            //
            // Arguments passed to the parser constructor are
            // forwarded to the message object. A single argument
            // is forwarded to the body constructor.
            //
            // We construct the dynamic body with a 1MB limit
            // to prevent vulnerability to buffer attacks.
            //
            parser_.emplace(
                std::piecewise_construct,
                std::make_tuple(),
                std::make_tuple(alloc_));

            http::async_read(
                socket_,
                buffer_,
                *parser_,
                [this](beast::error_code ec, std::size_t)
                {
                    if (ec)
                        accept();
                    else
                        process_request(parser_->get());
                });
        }

        void process_request(http::request<request_body_t, http::basic_fields<alloc_t>> const& req)
        {
            switch (req.method())
            {
            case http::verb::post:
            {
                auto target = req.target();
                // TODO match endpoints exactly
                if (target.rfind("tick") != std::string::npos) {
                    auto tick = json::parse(req.body()).template get<bot_api::GameState>();

                    auto commands = bot.Tick(tick);
                    auto response_json = json(commands).dump();

                    send_json(response_json);
                }
                else if (target.rfind("hello") != std::string::npos) {
                    auto mapInfo = json::parse(req.body()).template get<bot_api::MapInfo>();

                    auto decks = bot.DecksForMap(mapInfo);
                    auto response = bot_api::AiForMap();
                    response.name = bot.Name;
                    response.decks = std::move(decks);
                    auto response_json = json(response).dump();

                    send_json(response_json);
                }
                else if (target.rfind("prepare") != std::string::npos) {
                    auto prepare = json::parse(req.body()).template get<bot_api::Prepare>();

                    auto decks = bot.DecksForMap(prepare.map_info);
                    for (auto& d : decks)
                    {
                        if (d.name == prepare.deck) {
                            bot.PrepareForBattle(prepare.map_info, d);
                            send_json("");
                            return;
                        }
                    }
                    send_json("Deck not found");
                }
                else if (target.rfind("start") != std::string::npos) {
                    auto start = json::parse(req.body()).template get<bot_api::GameStartState>();

                    bot.MatchStart(start);
                    send_json("");
                }
                else if (target.rfind("end") != std::string::npos) {
                    // do nothing
                }
                break;
            }

            default:
                // We return responses indicating an error if
                // we do not recognize the request method.
                send_bad_response(
                    http::status::bad_request,
                    "Invalid request-method '" + std::string(req.method_string()) + "'\r\n");
                break;
            }
        }

        void send_bad_response(
            http::status status,
            std::string const& error)
        {
            string_response_.emplace(
                std::piecewise_construct,
                std::make_tuple(),
                std::make_tuple(alloc_));

            string_response_->result(status);
            string_response_->keep_alive(false);
            string_response_->set(http::field::server, "Beast");
            string_response_->set(http::field::content_type, "text/plain");
            string_response_->body() = error;
            string_response_->prepare_payload();

            string_serializer_.emplace(*string_response_);

            http::async_write(
                socket_,
                *string_serializer_,
                [this](beast::error_code ec, std::size_t)
                {
                    socket_.shutdown(tcp::socket::shutdown_send, ec);
                    string_serializer_.reset();
                    string_response_.reset();
                    accept();
                });
        }

        void send_json(std::string jsonText)
        {

            string_response_.emplace(
                std::piecewise_construct,
                std::make_tuple(),
                std::make_tuple(alloc_));

            string_response_->result(http::status::ok);
            string_response_->keep_alive(false);
            string_response_->set(http::field::server, "Beast");
            string_response_->set(http::field::content_type, "application/json");
            string_response_->body() = std::move(jsonText);
            string_response_->prepare_payload();

            string_serializer_.emplace(*string_response_);

            http::async_write(
                socket_,
                *string_serializer_,
                [this](beast::error_code ec, std::size_t)
                {
                    socket_.shutdown(tcp::socket::shutdown_send, ec);
                    string_response_.reset();
                    accept();
                });
        }

        void check_deadline()
        {
            // The deadline may have moved, so check it has really passed.
            if (request_deadline_.expiry() <= std::chrono::steady_clock::now())
            {
                // Close socket to cancel any outstanding operation.
                socket_.close();

                // Sleep indefinitely until we're given a new deadline.
                request_deadline_.expires_at(
                    (std::chrono::steady_clock::time_point::max)());
            }

            request_deadline_.async_wait(
                [this](beast::error_code)
                {
                    check_deadline();
                });
        }
    };

    static void run(IBotImp& bot, unsigned short port)
    {
        try
        {
            auto const address = net::ip::make_address("127.0.0.1");

            net::io_context ioc{ 1 };
            tcp::acceptor acceptor{ ioc, {address, port} };

            auto worker = http_worker(acceptor, bot);
            worker.start();
            std::cout << "running..." << std::endl;
            ioc.run();
            //for (;;) ioc.poll();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

}
}