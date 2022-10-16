using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Google.Protobuf.Protocol;

public class PlayerController : CreatureController
{

    protected Coroutine _coSkill;

    // 플레이어 초기화
    protected override void Init()
    {
        base.Init();
    }

    // 플레이어 업데이트
    protected override void UpdateController()
    {
        base.UpdateController();
    }
    [SerializeField]
    bool _rangedSkill = false; // 활이냐 근접이냐

    // 플레이어 애니메이션 재생
    protected override void UpdateAnimation()
    {
        // 처음상태에서 실행안되게
        if (_animator == null || _sprite == null) return;
        if (State == CreatureState.Idle)
        {
            switch (Dir)
            {
                case MoveDir.Up:
                    _animator.Play("IDLE_BACK");
                    _sprite.flipX = false;
                    break;
                case MoveDir.Down:
                    _animator.Play("IDLE_FRONT");
                    _sprite.flipX = false;
                    break;
                case MoveDir.Left:
                    _animator.Play("IDLE_RIGHT");
                    _sprite.flipX = true;
                    break;
                default:
                    _animator.Play("IDLE_RIGHT");
                    _sprite.flipX = false;
                    break;
            }
        }
        else if (State == CreatureState.Moving)
        {
            switch (Dir)
            {
                case MoveDir.Up:
                    _animator.Play("WALK_BACK");
                    _sprite.flipX = false;
                    break;
                case MoveDir.Down:
                    _animator.Play("WALK_FRONT");
                    _sprite.flipX = false;
                    break;
                case MoveDir.Left:
                    _animator.Play("WALK_RIGHT");
                    _sprite.flipX = true; // true일경우 x축을 flip(뒤집다)!
                    break;
                case MoveDir.Right:
                    _animator.Play("WALK_RIGHT");
                    _sprite.flipX = false;
                    break;

            }
        }
        else if (State == CreatureState.Skill)
        {
            switch (Dir)
            {
                case MoveDir.Up:
                    _animator.Play(_rangedSkill ? "ATTACK_WEAPON_BACK" : "ATTACK_BACK");
                    _sprite.flipX = false;
                    break;
                case MoveDir.Down:
                    _animator.Play(_rangedSkill ? "ATTACK_WEAPON_FRONT" : "ATTACK_FRONT");
                    _sprite.flipX = false;
                    break;
                case MoveDir.Left:
                    _animator.Play(_rangedSkill ? "ATTACK_WEAPON_RIGHT" : "ATTACK_RIGHT");
                    _sprite.flipX = true;
                    break;
                default:
                    _animator.Play(_rangedSkill ? "ATTACK_WEAPON_RIGHT" : "ATTACK_RIGHT");
                    _sprite.flipX = false;
                    break;
            }
        }
        else { }
    }

    public override void UseSkill(int skillId)
    {
        if (skillId == 1)
        {
            _coSkill = StartCoroutine("CoStartPunch");
        }
        else if(skillId == 2)
        {
            _coSkill = StartCoroutine("CoStartShootArrow");
        }
    }

    // 펀치 코루틴
    IEnumerator CoStartPunch()
    {
        // 피격 판정은 서버에서

        // 쿨타임
        _rangedSkill = false;
        State = CreatureState.Skill;
        yield return new WaitForSeconds(0.3f);
        State = CreatureState.Idle;
        _coSkill = null; 
        CheckUpdateFlag();
    }
    // 활쏘기 코루틴
    IEnumerator CoStartShootArrow()
    {
        // 쿨타임
        _rangedSkill = true;
        State = CreatureState.Skill;
        yield return new WaitForSeconds(0.3f);
        State = CreatureState.Idle;
        _coSkill = null; 
        CheckUpdateFlag();
    }

    public override void OnDamaged()
    {
        //Debug.Log("PlayerHIT!");
    }

    protected virtual void CheckUpdateFlag()
    {
        
    }
    public override void SynkPos()
    {
        Vector3 destPos = Managers.Map.CurrentGrid.CellToWorld(CellPos) + _sprightCorrection; // 오차값
        transform.position = destPos;
    }
    public override void OnDead()
    {
        base.OnDead();

        Managers.Object.Remove(Id);
        Managers.Resource.Destroy(gameObject);
    }
}
