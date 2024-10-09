#include <iostream>
#include "../API/json.hpp"
#include "../API/Types.hpp"
#include "../API/Helpers.hpp"
#include "../API/DebugPrint.hpp"

#include "../API/boost_wrapper.hpp"

using json = nlohmann::json;

//   /AI: add CppExampleBot TestDeck 4

class ExampleBot: public api::IBotImp
{
public:
	ExampleBot() : api::IBotImp("CppExampleBot"), myId{}, myTeamId{}, oponents{}, myStart{} { }
	~ExampleBot() override = default;
	std::vector<api::Deck> DecksForMap(const api::MapInfo& mapInfo) override;
	void PrepareForBattle(const api::MapInfo& mapInfo, const api::Deck& deck) override {
		std::cout << "Prepare for: " << mapInfo.map << std::endl;
		oponents.clear();
	}
	void MatchStart(const api::GameStartState& state) override;
	std::vector<api::Command> Tick(const api::GameState& state) override;

	const std::string Name;
private:
	api::EntityId myId;
	uint8_t myTeamId;
	std::vector<api::EntityId> oponents;
	api::Position2D myStart;
};

void run_ExampleBot(unsigned short port)
{
	auto bot = ExampleBot();

	api::run(bot, port);
}


std::vector<api::Deck> ExampleBot::DecksForMap(const api::MapInfo& mapInfo) {
	std::cout << "Can I play: " << mapInfo.map << std::endl;
	auto v = std::vector<api::Deck>();
	auto deck = api::Deck();
	deck.name = "TestDeck";
	deck.cover_card_index = 0;
	deck.cards[0] = api::CardIdWithUpgrade(card_templates::MasterArchers, api::Upgrade_U3);
	v.push_back(deck);

	return v;
}
void ExampleBot::MatchStart(const api::GameStartState& state) {
	myId = state.your_player_id;
	std::cout << "My id is " << myId << ", I own: " << std::endl;
	auto me = std::lower_bound(state.players.begin(), state.players.end(), myId,
		[](const api::MatchPlayer& p, const api::EntityId myId) { return p.entity.id < myId; });
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
			myStart = api::to2D(e.entity.position);
		}
	}
}
std::vector<api::Command> ExampleBot::Tick(const api::GameState& state) {
	/*/ example how to print WideLine DamageArea
	if (!state.entities.ability_world_objects.empty()) {
		std::cout << "tick: " << state.current_tick << std::endl;
		for (auto &a : state.entities.ability_world_objects)
		{
			std::cout << "  entity: " << a.entity.id << std::endl;

			for (auto& e : a.entity.effects)
			{
				std::cout << "    effect: " << e.id << std::endl;
				//auto effect_json = json(e).dump();
				//std::cout << "      " << effect_json << std::endl;
				if (e.specific.v.index() == 1) {
					auto& specific = std::get<api::AbilityEffectSpecificDamageArea>(e.specific.v);
					if (specific.shape.v.index() == 4) {
						auto& line = std::get<api::AreaShapeWideLine>(specific.shape.v);
						std::cout << "      start: "
							<< line.start.x << " / " << line.start.y
							<< " end: "
							<< line.end.x << " / " << line.end.y
							<< " width: " << line.width
							<< std::endl;
					}
					//else { std::cout << "      not AreaShapeWideLine" << std::endl; }
				}
				//else { std::cout << "      not AbilityEffectSpecificDamageArea" << std::endl; }
			}

			std::cout << std::endl;
		}
	}// */

	//std::cout << "tick " << state.current_tick << " entities count: " << state.entities.size() << std::endl;
	for(auto& r : state.rejected_commands)
	{
		if (r.player == myId)
			std::cout << "My command was rejected, because: " << Debug(r.reason);
	}

	auto me = std::lower_bound(state.players.begin(), state.players.end(), myId,
		[](const api::PlayerEntity& p, const api::EntityId myId) { return p.id < myId; });
	auto myPower = me->power;

	auto myArmy = std::vector<api::EntityId>();
	auto target = api::EntityId();
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

	auto v = std::vector<api::Command>();
	if (myPower >= 50.f) {
		auto spawn = api::CommandProduceSquad();
		spawn.card_position = 0;
		spawn.xy = myStart;
		v.push_back(api::Command(spawn));
	}
	if (!myArmy.empty() && target != 0) {
		auto attack = api::CommandGroupAttack();
		attack.squads = std::move(myArmy);
		attack.target_entity_id = target;
		v.push_back(api::Command(attack));
	}


	return v;
}