#include "Shaders/common.h"
#include "Shaders/Math.h"
#include "contract.h"

export void Ctor(Rewards::Params& params)
{
	const char* meta = "reward token";
	params.aid = Env::AssetCreate(meta, sizeof(meta) - 1);
	Env::Halt_if(!params.aid);
	Env::SaveVar_T(0, params);
}

export void Dtor()
{
	Env::DelVar_T(0);
}

export void Method_2(const Rewards::TakeFreeTokensParams& params)
{
	Rewards::Params init_params;
	Env::LoadVar_T(0, init_params);

	Rewards::AccountData ac{};
	bool is_loaded = Env::LoadVar_T(params.receiver, ac);

	Height cur_height = Env::get_Height();
	
	if (!is_loaded || cur_height - ac.last_time_received > init_params.free_tokens_period) {
		Env::AssetEmit(init_params.aid, init_params.free_tokens_amount, true);
		Strict::Add(ac.balance, init_params.free_tokens_amount);
		Env::SaveVar_T(params.receiver, ac);
		Env::FundsUnlock(init_params.aid, init_params.free_tokens_amount);
	}
	Env::AddSig(params.receiver);
}

void Method_3(const Rewards::GiveRewardsParams& params)
{
	
}
