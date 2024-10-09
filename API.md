# SR Bot API description:
Each bot needs to provide 4 endpoint accessible trough http.
For full description check [Rust API.md](https://gitlab.com/skylords-reborn/skylords-reborn-bot-api-rust/-/blob/main/API.md)


## boost_wrapper.hpp
This C++ API wrapper implements the boilerplate for the API and simplifies it to implementation of ``IBotImp`` *interface* class.
Users that decide to use this wrapper to not need to implement any part of HTTP communication themselves.


#### POST ``hello`` endpoint
- ``DecksForMap`` function call with ``MapInfo`` as parameter.
- it also reads ``Name`` to fill in that parameter.

#### POST ``prepare`` endpoint
- ``DecksForMap`` function call with ``MapInfo`` as parameter as with the ``hello`` endpoint.
- It finds the requested deck, and calls ``PrepareForBattle`` with map, and deck information.

#### POST ``start`` endpoint
- ``MatchStart`` function call with the state.

#### POST ``tick`` endpoint
- ``Tick`` function call with the state.

#### GET ``end`` endpoint
- does nothing