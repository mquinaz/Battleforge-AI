#include <iostream>
#include "../API/json.hpp"
#include "../API/TypesCLike.hpp"
#include "../API/HelpersCLike.hpp"

#include "../API/boost_wrapper.hpp"

using json = nlohmann::json;

//   /AI: add BasicBot RelaxDeck 4

class BasicBot: public capi::IBotImp
{
public:
	BasicBot() : capi::IBotImp("DumDumBot"), myId{}, myTeamId{}, oponents{}, myStart{} { }
	~BasicBot() override = default;
	std::vector<capi::Deck> DecksForMap(const capi::MapInfo& mapInfo) override;
	void PrepareForBattle(const capi::MapInfo& mapInfo, const capi::Deck& deck) override {
		std::cout << "Prepare for: " << mapInfo.map << std::endl;
		oponents.clear();
	}
	void MatchStart(const capi::GameStartState& state) override;
	std::vector<capi::Command> Tick(const capi::GameState& state) override;

	const std::string Name;
private:
	capi::EntityId myId;
	uint8_t myTeamId;
	std::vector<capi::EntityId> oponents;
	capi::Position2D myStart;
};

void run_BasicBot(unsigned short port)
{
	auto bot = BasicBot();

	capi::run(bot, port);
}


std::vector<capi::Deck> BasicBot::DecksForMap(const capi::MapInfo& mapInfo) {
	std::cout << "Can I play: " << mapInfo.map << std::endl;
	auto v = std::vector<capi::Deck>();
	auto deck = capi::Deck();
	deck.name = "RelaxDeck";
	deck.cover_card_index = 0;
	deck.cards[0] = capi::CardIdWithUpgrade(card_templates::MasterArchers, capi::Upgrade_U3);
	deck.cards[1] = capi::CardIdWithUpgrade(card_templates::AvatarofFrost, capi::Upgrade_U3);
	v.push_back(deck);

	return v;
}

void BasicBot::MatchStart(const capi::GameStartState& state) {
	myId = state.your_player_id;
	std::cout << "My id is " << myId << ", I own: " << std::endl;
	auto me = std::lower_bound(state.players.begin(), state.players.end(), myId,
		[](const capi::MatchPlayer& p, const capi::EntityId myId) { return p.entity.id < myId; });
	myTeamId = me->entity.team;
	for (auto& p : state.players) {
		if (p.entity.team != myTeamId) {
			oponents.push_back(p.entity.id);
		}
	}
	for (auto& e : state.entities.power_slots) {
		if (e.entity.player_entity_id == myId) {
			std::cout << "power slot " << e.entity.id << " at " << e.entity.position.x << " " << e.entity.position.z << std::endl;
		}
	}
	for (auto& e : state.entities.token_slots) {
		if (e.entity.player_entity_id == myId) {
			std::cout << "token slot " << e.entity.id << " at " << e.entity.position.x << " " << e.entity.position.z << std::endl;
			myStart = capi::to2D(e.entity.position);
		}
	}
}
std::vector<capi::Command> BasicBot::Tick(const capi::GameState& state) {
	//
	const capi::PlayerEntity* me = nullptr;
	for (auto i = 0; i < state.players.size(); i++)
	{
		if (state.players[i].id == myId) {
			me = &state.players[i];
			break;
		}
	}
	if (me == nullptr)
		throw std::runtime_error("Could not find my player entity");
	auto myPower = me->power;

	auto myArmy = std::vector<capi::EntityId>();
	auto target = capi::EntityId();
	for (auto& e : state.entities.squads) {
		if (e.entity.player_entity_id == myId) {
			myArmy.push_back(e.entity.id);
		}
	}
	for (auto& e : state.entities.token_slots) {
		for (auto oponent : oponents) {
			if (e.entity.player_entity_id == oponent) {
				target = e.entity.id;
			}
		}
	}

	auto v = std::vector<capi::Command>();
	if (myPower >= 50.f) {
		auto spawn = capi::CommandProduceSquad();
		spawn.card_position = 0;
		spawn.xy = myStart;
		v.push_back(capi::Command(std::move(spawn)));
	}
	if (!myArmy.empty() && target != 0) {
		auto attack = capi::CommandGroupAttack();
		attack.squads = std::move(myArmy);
		attack.target_entity_id = target;
		v.push_back(capi::Command(std::move(attack)));
	}


	return v;
}