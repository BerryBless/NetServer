using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Google.Protobuf.Protocol;

public class CreatureController : BaseController
{
    HpBar _hpBar;

    [SerializeField]
    protected Vector3 _sprightCorrection = new Vector3(0.5f, 0.5f);       // 캐릭터 스프라이트를 그리드안에 넣기위한 오차값


    [SerializeField]
    PositionInfo _positionInfo = new PositionInfo();

    public override StatInfo Stat
    {
        get { return base._stat; }
        set
        {
            if (base._stat.Equals(value)) return;
            base._stat = value;
            UpdateHpBar();
        }
    }
    public override int Hp
    {
        get { return base._stat.Hp; }
        set
        {
            base._stat.Hp = value;
            //_stat.Hp = Mathf.Max(value, 0);
            UpdateHpBar();
        }
    }

    // HP바 필요한 오브젝트 붙여주기 
    protected virtual void AddHpBar()
    {
        GameObject go = Managers.Resource.Instantiate("UI/HpBar", transform);
        go.transform.localPosition = new Vector3(0, 0.5f, 0);
        go.name = "HpBar";

        _hpBar = go.GetComponent<HpBar>();
        UpdateHpBar();
    }
    // 체력바 업데이트
    void UpdateHpBar()
    {
        if (_hpBar == null) return;

        float ratio = 0.0f;
        if (Stat.MaxHp > 0)
        {
            ratio = ((float)Hp / Stat.MaxHp);
        }
        _hpBar.SetHpBar(ratio);

    }


    // Start에 넣을꺼 자식에서 오버라이드 해주기
    protected override void Init()
    {
        base.Init();
        AddHpBar();
        transform.position += _sprightCorrection;//
    }

    // 으앙 쥬금
    public virtual void OnDead()
    {
        State = CreatureState.Dead;

        // 죽음 이펙트
        GameObject effect = Managers.Resource.Instantiate("Effect/DieEffect");
        effect.transform.position = transform.position;
        effect.GetComponent<Animator>().Play("START");
        GameObject.Destroy(effect, 0.5f);
    }
    // 싱크 맞춰주기
    public virtual void SynkPos()
    {
        Vector3 destPos = Managers.Map.CurrentGrid.CellToWorld(CellPos); // 포지션 변경
        transform.position = destPos;
    }

    // 스킬 사용 (행동)
    public virtual void UseSkill(int skillId)
    {

    }
}
