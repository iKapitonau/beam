#include "Shaders/common.h"
#include "contract.h"

#include <string>
#include <unordered_map>

enum Action {
	CREATE_CONTRACT,
	DESTROY_CONTRACT,
	INVALID_ACTION,
	TAKE_FREE_TOKENS,
	GIVE_REWARDS,
	SHOW_REWARDS,
};

static const size_t ACTION_SIZE = 16;

static std::unordered_map<const char*, Action> VALID_ACTIONS = {
	{"create_contract", CREATE_CONTRACT},
	{"destroy_contract", DESTROY_CONTRACT},
	{"take_free_tokens", TAKE_FREE_TOKENS},
	{"give_rewards", GIVE_REWARDS},
	{"show_rewards", SHOW_REWARDS},
};

void OnError(const char* msg)
{
    Env::DocAddText("error", msg);
}

void DeriveMyPk(PubKey& pubKey, const ContractID& cid)
{
    Env::DerivePk(pubKey, &cid, sizeof(cid));
}

void On_action_create_contract(const Height& free_tokens_period, const Height& free_tokens_amount, const Amount& max_reward, const ContractID& cid)
{
	Rewards::Params pars;
	pars.free_tokens_period = free_tokens_period;
	pars.free_tokens_amount = free_tokens_amount;
	pars.max_reward = max_reward;
	Env::GenerateKernel(nullptr, pars.METHOD, &pars, sizeof(pars), nullptr, 0, nullptr, 0, "create Rewards contract", 0);
}

void On_action_destroy_contract(const ContractID& cid)
{
	Env::GenerateKernel(&cid, 1, nullptr, 0, nullptr, 0, nullptr, 0, "destroy Rewards contract", 0);
}

void On_action_take_free_tokens(const ContractID& cid)
{
	Env::Key_T<uint8_t> k;
	k.m_Prefix.m_Cid = cid;
	k.m_KeyInContract = 0;

	Rewards::Params initial_params;
	if (!Env::VarReader::Read_T(k, initial_params))
		return OnError("failed to read initial params");

	Rewards::TakeFreeTokensParams params;
	DeriveMyPk(params.receiver, cid);

	SigRequest sig;
	sig.m_pID = &cid;
	sig.m_nID = sizeof(cid);

	FundsChange fc;
	fc.m_Amount = initial_params.free_tokens_amount;
	fc.m_Aid = initial_params.aid;
	fc.m_Consume = 0;

	Env::GenerateKernel(&cid, Rewards::TakeFreeTokensParams::METHOD, &params, sizeof(params), &fc, 1, &sig, 1, "take free tokens from rewards contract", 0);
}

void On_action_show_rewards(const ContractID& cid)
{
	PubKey pubKey;
    DeriveMyPk(pubKey, cid);

	Env::Key_T<PubKey> k;
	k.m_Prefix.m_Cid = cid;
	k.m_KeyInContract = pubKey;

	Rewards::AccountData ac;
	if (Env::VarReader::Read_T(k, ac)) {
		Env::DocAddNum64("rewards", ac.rewards);
	} else {
		Env::DocAddNum64("rewards", 0);
	}
}

export void Method_0()
{
}

export void Method_1()
{
	char action[ACTION_SIZE];

	if (!Env::DocGetText("action", action, sizeof(action)))
		return OnError("Action not specified");

	Action action_id = (VALID_ACTIONS.find(action) == VALID_ACTIONS.end() ?
		Action::INVALID_ACTION : VALID_ACTIONS[action]);

	ContractID cid;
	Env::DocGet("cid", cid); 
	switch(action_id) {
		case Action::CREATE_CONTRACT: {
			Height free_tokens_period;
			Env::DocGet("free_tokens_period", free_tokens_period);
			Amount free_tokens_amount;
			Env::DocGet("free_tokens_amount", free_tokens_amount);
			Amount max_reward;
			Env::DocGet("max_reward", max_reward);
			On_action_create_contract(free_tokens_period, free_tokens_amount, max_reward, cid);
			break;
		}
		case Action::DESTROY_CONTRACT:
			On_action_destroy_contract(cid);
			break;
		case Action::TAKE_FREE_TOKENS:
			On_action_take_free_tokens(cid);
			break;
		case Action::GIVE_REWARDS:
			break;
		case Action::SHOW_REWARDS: {
			On_action_show_rewards(cid);
			break;
		}
		default:
			return OnError("invalid Action");
	}
}
