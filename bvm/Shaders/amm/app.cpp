#include "../common.h"
#include "../app_common_impl.h"
#include "contract.h"

#define Amm_admin_create(macro)
#define Amm_admin_view(macro)
#define Amm_admin_destroy(macro) macro(ContractID, cid)
#define Amm_admin_pools_view(macro) macro(ContractID, cid)

#define Amm_admin_pool_view(macro) \
    macro(ContractID, cid) \
    macro(AssetID, aid1) \
    macro(AssetID, aid2)

#define Amm_admin_pool_create(macro) Amm_admin_pool_view(macro)
#define Amm_admin_pool_destroy(macro) Amm_admin_pool_view(macro)


#define AmmRole_admin(macro) \
    macro(admin, view) \
    macro(admin, create) \
    macro(admin, destroy) \
    macro(admin, pool_view) \
    macro(admin, pools_view) \
    macro(admin, pool_create) \
    macro(admin, pool_destroy)


#define Amm_user_trade(macro) \
    macro(ContractID, cid) \
    macro(AssetID, aidBuy) \
    macro(AssetID, aidPay) \
    macro(Amount, valBuy) \
    macro(uint32_t, bPredictOnly)


#define AmmRole_user(macro) \
    macro(user, trade) \


#define AmmRoles_All(macro) \
    macro(admin) \
    macro(user)

namespace Amm {

BEAM_EXPORT void Method_0()
{
    // scheme
    Env::DocGroup root("");

    {   Env::DocGroup gr("roles");

#define THE_FIELD(type, name) Env::DocAddText(#name, #type);
#define THE_METHOD(role, name) { Env::DocGroup grMethod(#name);  Amm_##role##_##name(THE_FIELD) }
#define THE_ROLE(name) { Env::DocGroup grRole(#name); AmmRole_##name(THE_METHOD) }
        
        AmmRoles_All(THE_ROLE)
#undef THE_ROLE
#undef THE_METHOD
#undef THE_FIELD
    }
}

#define THE_FIELD(type, name) const type& name,
#define ON_METHOD(role, name) void On_##role##_##name(Amm_##role##_##name(THE_FIELD) int unused = 0)

void OnError(const char* sz)
{
    Env::DocAddText("error", sz);
}

ON_METHOD(admin, view)
{
    EnumAndDumpContracts(s_SID);
}

ON_METHOD(admin, create)
{
    Env::GenerateKernel(nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0, "create Amm contract", 0);
}

ON_METHOD(admin, destroy)
{
    Env::GenerateKernel(&cid, 1, nullptr, 0, nullptr, 0, nullptr, 0, "destroy Amm contract", 0);
}

bool SetKey(Pool::ID& pid, AssetID aid1, AssetID aid2)
{
    if (aid1 < aid2)
    {
        pid.m_Aid1 = aid1;
        pid.m_Aid2 = aid2;
    }
    else
    {
        if (aid1 == aid2)
        {
            OnError("assets must be different");
            return false;
        }

        pid.m_Aid1 = aid2;
        pid.m_Aid2 = aid1;
    }

    return true;
}

bool ReadPool(Pool& p, const ContractID& cid, const Pool::ID& pid)
{
    Env::Key_T<Pool::Key> key;
    _POD_(key.m_Prefix.m_Cid) = cid;
    key.m_KeyInContract.m_ID = pid;

    return Env::VarReader::Read_T(key, p);
}

void PrintPool(const Pool& p)
{
    Env::DocAddNum("aidCtl", p.m_aidCtl);
    Env::DocAddNum("ctl", p.m_Totals.m_Ctl);
    Env::DocAddNum("tok1", p.m_Totals.m_Tok1);
    Env::DocAddNum("tok2", p.m_Totals.m_Tok2);
}

ON_METHOD(admin, pools_view)
{
    Env::Key_T<Pool::Key> key1, key2;
    _POD_(key1.m_Prefix.m_Cid) = cid;
    _POD_(key2.m_Prefix.m_Cid) = cid;
    _POD_(key1.m_KeyInContract.m_ID).SetZero();
    _POD_(key2.m_KeyInContract.m_ID).SetObject(0xff);

    Env::DocArray gr("res");

    for (Env::VarReader r(key1, key2); ; )
    {
        Pool p;
        if (!r.MoveNext_T(key1, p))
            break;

        Env::DocGroup gr1("");

        Env::DocAddNum("aid1", key1.m_KeyInContract.m_ID.m_Aid1);
        Env::DocAddNum("aid2", key1.m_KeyInContract.m_ID.m_Aid2);
        PrintPool(p);
    }
}

ON_METHOD(admin, pool_view)
{
    Pool::ID pid;
    if (!SetKey(pid, aid1, aid2))
        return;

    Pool p;
    if (ReadPool(p, cid, pid))
    {
        Env::DocGroup gr("res");
        PrintPool(p);
    }
    else
        OnError("no such a pool");
}

static const Amount g_DepositCA = 3000 * g_Beam2Groth; // 3K beams

ON_METHOD(admin, pool_create)
{
    Method::CreatePool arg;
    if (!SetKey(arg.m_PoolID, aid1, aid2))
        return;

    Pool p;
    if (ReadPool(p, cid, arg.m_PoolID))
        return OnError("already exists");

    FundsChange fc;
    fc.m_Aid = 0;
    fc.m_Consume = 1;
    fc.m_Amount = g_DepositCA;


    Env::GenerateKernel(&cid, arg.s_iMethod, &arg, sizeof(arg), &fc, 1, nullptr, 0, "Amm pool create", 0);
}

ON_METHOD(admin, pool_destroy)
{
    Method::DeletePool arg;
    if (!SetKey(arg.m_PoolID, aid1, aid2))
        return;

    Pool p;
    if (!ReadPool(p, cid, arg.m_PoolID))
        return OnError("no such a pool");

    if (p.m_Totals.m_Ctl)
        return OnError("pool is non-empty");

    FundsChange fc;
    fc.m_Aid = 0;
    fc.m_Consume = 0;
    fc.m_Amount = g_DepositCA;


    Env::GenerateKernel(&cid, arg.s_iMethod, &arg, sizeof(arg), &fc, 1, nullptr, 0, "Amm pool destroy", 0);
}

ON_METHOD(user, trade)
{
}

#undef ON_METHOD
#undef THE_FIELD

BEAM_EXPORT void Method_1() 
{
    Env::DocGroup root("");

    char szRole[0x10], szAction[0x10];

    if (!Env::DocGetText("role", szRole, sizeof(szRole)))
        return OnError("Role not specified");

    if (!Env::DocGetText("action", szAction, sizeof(szAction)))
        return OnError("Action not specified");

    const char* szErr = nullptr;

#define PAR_READ(type, name) type arg_##name; Env::DocGet(#name, arg_##name);
#define PAR_PASS(type, name) arg_##name,

#define THE_METHOD(role, name) \
        if (!Env::Strcmp(szAction, #name)) { \
            Amm_##role##_##name(PAR_READ) \
            On_##role##_##name(Amm_##role##_##name(PAR_PASS) 0); \
            return; \
        }

#define THE_ROLE(name) \
    if (!Env::Strcmp(szRole, #name)) { \
        AmmRole_##name(THE_METHOD) \
        return OnError("invalid Action"); \
    }

    AmmRoles_All(THE_ROLE)

#undef THE_ROLE
#undef THE_METHOD
#undef PAR_PASS
#undef PAR_READ

    OnError("unknown Role");
}

} // namespace Amm
