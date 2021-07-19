#include "Shaders/common.h"
#include "contract.h"

#include <algorithm>
#include <utility>
#include <vector>

#define ACTION_SIZE	16

using Action_func_t = void (*)(const ContractID&);

void OnError(const char* msg)
{
    Env::DocAddText("error", msg);
}

void DeriveMyPk(PubKey& pubKey, const ContractID& cid)
{
    Env::DerivePk(pubKey, &cid, sizeof(cid));
}

void On_action_create_contract(const ContractID& cid)
{
	Rewards::Params pars;
	Env::DocGet("free_tokens_period", pars.free_tokens_period);
	Env::DocGet("free_tokens_amount", pars.free_tokens_amount);
	Env::DocGet("max_reward", pars.max_reward);

//	FundsChange fc;
//	fc.m_Aid = 0;
//	fc.m_Amount = 5000000000ll;
//	fc.m_Consume = 1;
	Env::GenerateKernel(nullptr, pars.METHOD, &pars, sizeof(pars), nullptr, 0, /*&fc, 1,*/ nullptr, 0, "create Rewards contract", 0);
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
	const std::vector<std::pair<const char *, Action_func_t>> VALID_ACTIONS = {
		{"create_contract", On_action_create_contract},
		{"destroy_contract", On_action_destroy_contract},
		{"take_free_tokens", On_action_take_free_tokens},
		//{"give_rewards", On_action_give_rewards},
		{"show_rewards", On_action_show_rewards},
	};

	char action[ACTION_SIZE];

	if (!Env::DocGetText("action", action, sizeof(action)))
		return OnError("Action not specified");

	auto it = std::find_if(VALID_ACTIONS.begin(), VALID_ACTIONS.end(), [&action](const auto& p) {
		return !strcmp(action, p.first);
	});

	if (it != VALID_ACTIONS.end()) {
		ContractID cid;
		Env::DocGet("cid", cid); 
		it->second(cid);
	} else {
		OnError("invalid Action");
	}
}
