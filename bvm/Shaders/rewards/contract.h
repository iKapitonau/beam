#pragma once

namespace Rewards {
	static const ShaderID s_SID = {0x3c,0x84,0x18,0x0a,0xd4,0x3b,0xca,0x02,0x9f,0x2b,0x62,0x18,0x73,0xae,0xb8,0xe3,0x06,0xfa,0x54,0xc1,0xdb,0x60,0x26,0x8e,0x85,0x55,0x9d,0xc8,0x68,0xa1,0x83,0x60};

#pragma pack(push, 1)

	struct Params {
		static const uint32_t METHOD = 0;
		Height backlog_max_height;
		Height free_tokens_period;
		Amount free_tokens_amount;
		Amount max_reward;
		AssetID aid;
	};

	struct TakeFreeTokensParams {
		static const uint32_t METHOD = 2;
		PubKey receiver;
	};

	struct GiveRewardsParams {
		static const uint32_t METHOD = 3;
		PubKey receiver;
		PubKey sender;
		Amount amount;
	};

	struct AccountData {
		Amount balance;
		Amount rewards;
		Height last_time_received;
	};

#pragma pack(pop)
}
