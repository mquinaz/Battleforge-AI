![Skylords Reborn Logo](https://gitlab.com/skylords-reborn/rust-libraries/-/blob/main/images/skylords_reborn_logo.png)

# Skylords Reborn Bot API C++

This repository contains an interface for bots to interact with the game BattleForge.

## Repo Structure

There are 2 folder, and [API.md](./API.md).

### Example

This folder contains example bot implementations project. We recommend using this project as base for your bot, and adding your own bot implementation.

### API

This folder contains the data types needed for communication with the game.
These are only header files.

It also contains ``boost_wrapper.hpp`` that simplifies the implementation to single *interface* **class** implementation.
To use this wrapper you need to have boost in path ``BOOST_ROOT`` (environment variable pointo to a folder containing folder `boost`), or edit the project to include your folder.

### API.md

Explains how the api should work, and how the example api wrapper makes it easier to work with.

## You want to check other language?
- [Rust](https://gitlab.com/skylords-reborn/skylords-reborn-bot-api-rust)
- [C#](https://gitlab.com/skylords-reborn/skylords-reborn-bot-api-c-sharp)
- [C++](https://gitlab.com/skylords-reborn/skylords-reborn-bot-api-cplusplus)
- [Python](https://gitlab.com/skylords-reborn/skylords-reborn-bot-api-python)
- [Java](https://github.com/TheMelmacian/Skylords-Reborn-Bot-Api-Java)

## How to Contribute

Thank you for your interest in contributing to the Skylords Reborn Bot API C++ of Skylords Reborn.

To contribute to the source code, please follow these steps:

- Fork the repository and clone it locally.
- Create a short and descriptive branch in your fork.
- Commit and push your changes to that branch.
- Create a Merge Request to `main` from the repository. 
- Provide clear and concise titles to your merge requests. A merge request is not obliged to have an issue for now. Choose an appropriate template when creating your merge requests.
- When creating a merge request (MR), please mention the related issue, the changes made, and how to test it. If you have provided a detailed first commit message, these points will already be covered.
- Use the "Draft" status on MRs when relevant.
- Every merge request must be reviewed by at least one person before it is merged.
- Depending on the number of contributions to this project, this process may be refined in the future.

For any questions, please contact [Kubik](https://forum.skylords.eu/index.php?/profile/19915-kubik/), or check out [discord](https://discord.com/channels/1158440761424089212/1158442837113831476).

## License

This project is open source and available under Boost Software License 1.0. See [LICENSE](./LICENSE) for more information.

## Disclosure
EA has not endorsed and does not support this product.