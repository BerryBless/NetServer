using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Google.Protobuf.Protocol;

public class BaseController : MonoBehaviour
{
    public int Id { get; set; }                                 // 식별 ID

    protected bool _updated = false; // 업데이트가 되었느냐 더티플레그

    [SerializeField]
    PositionInfo _positionInfo = new PositionInfo();
    public PositionInfo PosInfo
    {
        get { return _positionInfo; }
        set
        {
            if (_positionInfo.Equals(value)) return;

            // 갱신 잘되게
            CellPos = new Vector3Int(value.PosX, value.PosY, 0);
            State = value.State;
            Dir = value.MoveDir;
        }
    }

    public Vector3Int CellPos
    {
        get { return new Vector3Int(PosInfo.PosX, PosInfo.PosY, 0); }
        set
        {
            if (PosInfo.PosX == value.x && PosInfo.PosY == value.y) return;
            PosInfo.PosX = value.x;
            PosInfo.PosY = value.y;

            _updated = true; // 더러워

        }
    }                                // 크리쳐의 현재 샐포지션
    protected Animator _animator;                               // 크리쳐의 애니메이터
    protected SpriteRenderer _sprite;                           // 크리쳐의 스프라이트
    // stat관련
    [SerializeField]
    protected StatInfo _stat = new StatInfo();
    public virtual StatInfo Stat
    {
        get { return _stat; }
        set
        {
            if (_stat.Equals(value)) return;
            _stat = value;
        }
    }
    public float Speed { get { return _stat.Speed; } set { _stat.Speed = value; } }
    public virtual int Hp
    {
        get { return _stat.Hp; }
        set
        {
            _stat.Hp = Mathf.Max(value, 0);
        }
    }
    [SerializeField]
    public virtual CreatureState State
    {
        get { return PosInfo.State; }
        set
        {
            if (PosInfo.State.Equals(value)) return;


            PosInfo.State = value;

            UpdateAnimation();
            _updated = true; // 더러워

        }
    }                       // 크리쳐의 상태변화에 따른 애니메이션 호출

    // 이동!
    [SerializeField]
    public MoveDir Dir
    {
        get { return PosInfo.MoveDir; }
        set
        {
            if (PosInfo.MoveDir.Equals(value)) return;

            PosInfo.MoveDir = value;
            // 마지막으로 바라본 방향

            UpdateAnimation();
            _updated = true; // 더러워
        }
    }                                       // 크리쳐 방향변화에 따른 애니메이션 선택

    // 마지막으로 본 방향의 한칸앞 셀포지션 리턴
    public Vector3Int GetFrontCellPos()
    {
        Vector3Int frontPos = CellPos;

        switch (Dir)
        {
            case MoveDir.Up:
                frontPos += Vector3Int.up;
                break;
            case MoveDir.Down:
                frontPos += Vector3Int.down;
                break;
            case MoveDir.Left:
                frontPos += Vector3Int.left;
                break;
            case MoveDir.Right:
                frontPos += Vector3Int.right;
                break;
        }
        return frontPos;
    }

    // Dir 벡터에 따른 방향 정하기
    public MoveDir GetDirFromVector(Vector3Int dir)
    {
        if (dir.x > 0)
            return MoveDir.Right;
        else if (dir.x < 0)
            return MoveDir.Left;
        else if (dir.y > 0)
            return MoveDir.Up;
        return MoveDir.Down;
    }

    // State에 따른 애니메이션 관리
    protected virtual void UpdateAnimation()
    {
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
                    _animator.Play("ATTACK_BACK");
                    _sprite.flipX = false;
                    break;
                case MoveDir.Down:
                    _animator.Play("ATTACK_FRONT");
                    _sprite.flipX = false;
                    break;
                case MoveDir.Left:
                    _animator.Play("ATTACK_RIGHT");
                    _sprite.flipX = true;
                    break;
                default:
                    _animator.Play("ATTACK_RIGHT");
                    _sprite.flipX = false;
                    break;
            }
        }
        else { }
    }

    // Start is called before the first frame update
    void Start()
    {
        Init();
    }

    // Update is called once per frame
    void Update()
    {
        UpdateController();
    }

    // Start에 넣을꺼 자식에서 오버라이드 해주기
    protected virtual void Init()
    {
        _sprite = GetComponent<SpriteRenderer>();
        _animator = GetComponent<Animator>();
        Vector3 pos = Managers.Map.CurrentGrid.CellToWorld(CellPos);
        transform.position = pos;

        UpdateAnimation();

    }

    // Update에 넣을꺼 자식에서 오버라이드 해주기
    protected virtual void UpdateController()
    {
        // 상태에 따른 업데이트 관리
        switch (State)
        {
            case CreatureState.Idle:
                UpdateIdle();
                break;
            case CreatureState.Moving:
                UpdateMoving();
                break;
            case CreatureState.Skill:
                UpdateSkill();
                break;
            case CreatureState.Dead:
                UpdateDead();
                break;
        }

    }

    /* 업데이트 오버라이드 할것들 */
    // 아이들 상태
    protected virtual void UpdateIdle()
    {
    }
    // 클라이언트에서 스르륵 움직이는 것처럼 보이게
    protected virtual void UpdateMoving()
    {
        Vector3 destPos = Managers.Map.CurrentGrid.CellToWorld(CellPos) + new Vector3(0.5f, 0.5f, 0); // 최종 목적지
        Vector3 moveDir = destPos - transform.position; // 이동에 필요한 방향백터 

        // 도착 여부 체크 스피드 크면 버그걸림
        float dist = moveDir.magnitude;
        if (dist < Speed * Time.deltaTime)
        {
            transform.position = destPos;
            MoveToNextPos();

        }
        // 계속 움직이기
        else
        {
            transform.position += moveDir.normalized * Speed * Time.deltaTime;
            State = CreatureState.Moving;
        }

    }
    // 스킬 쓰는 상태
    protected virtual void UpdateSkill()
    {

    }
    // 죽는 상태
    protected virtual void UpdateDead()
    {

    }

    // 판정상으로 움직임
    protected virtual void MoveToNextPos()
    {

    }

    // 데미지 받을때
    public virtual void OnDamaged()
    {

    }
}
