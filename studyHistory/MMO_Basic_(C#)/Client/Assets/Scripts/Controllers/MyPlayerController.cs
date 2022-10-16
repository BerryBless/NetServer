using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Google.Protobuf.Protocol;

public class MyPlayerController : PlayerController
{
    bool _moveKeyPressed = false;

    public int WeaponDamage { get; private set; }
    public int ArmorDefence { get; private set; }

    // 플레이어 초기화
    protected override void Init()
    {
        base.Init();
        RefreshAdditionalStat();
    }

    // 플레이어 업데이트
    protected override void UpdateController()
    {
        GetUIKeyInput();
        // State에 따라 적절하게 입력 받기
        switch (State)
        {
            case CreatureState.Idle:
                GetDirInput();
                break;
            case CreatureState.Moving:
                GetDirInput();
                break;
            case CreatureState.Skill:
                break;
            case CreatureState.Dead:
                break;
        }

        base.UpdateController();
    }
    // 카메라 따라가게
    private void LateUpdate()
    {
        Camera.main.transform.position = new Vector3(transform.position.x, transform.position.y, -10);
    }

    // 판정상으로 움직임
    protected override void MoveToNextPos()
    {
        /* 실제 움직이는 부분 */
        // 이동을 멈췄냐?
        if (_moveKeyPressed == false)
        {
            State = CreatureState.Idle;
            CheckUpdateFlag();
            return;
        }

        // 이동중 실제좌표 이동
        Vector3Int destPos = CellPos; // 목적지 좌표찍기

        switch (Dir)
        {
            case MoveDir.Up:
                destPos += Vector3Int.up;
                break;
            case MoveDir.Down:
                destPos += Vector3Int.down;
                break;
            case MoveDir.Left:
                destPos += Vector3Int.left;
                break;
            case MoveDir.Right:
                destPos += Vector3Int.right;
                break;
        }

        // 맵상에서 갈수있나
        if (Managers.Map.CanGo(destPos) == true)
        {
            // 충돌되는 오브젝트가 있나
            if (Managers.Object.FindCreatur(destPos) == null)
            {
                CellPos = destPos;
            }
        }
        /* 실제 움직이는 부분 끝 */

        CheckUpdateFlag();
    }

    // 인벤열기
    void GetUIKeyInput()
    {
        if (Input.GetKeyDown(KeyCode.I))
        {
            UI_GameScene gameSceneUI = Managers.UI.SceneUI as UI_GameScene;
            UI_Inventory invenUI = gameSceneUI.InvenUI;

            if (invenUI.gameObject.activeSelf)
                invenUI.gameObject.SetActive(false);
            else
            {
                invenUI.gameObject.SetActive(true);
                invenUI.RefreshUI();
            }
        }else if (Input.GetKeyDown(KeyCode.C))
        {
            UI_GameScene gameSceneUI = Managers.UI.SceneUI as UI_GameScene;
            UI_Stat statUI= gameSceneUI.StatUI;

            if (statUI.gameObject.activeSelf)
                statUI.gameObject.SetActive(false);
            else
            {
                statUI.gameObject.SetActive(true);
                statUI.RefreshUI();
            }
        }
    }


    // 움직일 키입력
    void GetDirInput()
    {
        _moveKeyPressed = true;

        float _axisV = Input.GetAxisRaw("Vertical");
        float _axisH = Input.GetAxisRaw("Horizontal");
        if (_axisV != 0)
        {
            Dir = _axisV > 0 ? MoveDir.Up : MoveDir.Down;
        }
        else if (_axisH != 0)
        {
            Dir = _axisH > 0 ? MoveDir.Right : MoveDir.Left;
        }
        else
        {
            _moveKeyPressed = false;
        }

    }

    // 가만히 있을때 스킬 버튼을 입력받나?
    protected override void UpdateIdle()
    {
        // 이동이냐?
        if (_moveKeyPressed)
            State = CreatureState.Moving;

        // 스킬이냐?
        if (Input.GetButton("Fire1") && _coSkillCoolTime == null)
        {
            //Debug.Log("SKILL !!");
            // 스킬 패킷보내기
            C_Skill skill = new C_Skill() { Info = new SkillInfo() };
            skill.Info.SkillId = 2; // TEST: 1번은 평타 2번은 화살
            Managers.Network.Send(skill);

            _coSkillCoolTime = StartCoroutine("SkillCoolTime", 0.3f);
        }
        // 스킬이냐?
        else if (Input.GetButton("Fire2") && _coSkillCoolTime == null)
        {
            //Debug.Log("SKILL !!");
            // 스킬 패킷보내기
            C_Skill skill = new C_Skill() { Info = new SkillInfo() };
            skill.Info.SkillId = 1; // TEST: 1번은 평타 2번은 화살
            Managers.Network.Send(skill);

            _coSkillCoolTime = StartCoroutine("SkillCoolTime", 0.3f);
        }
    }

    Coroutine _coSkillCoolTime;
    IEnumerator SkillCoolTime(float time)
    {
        yield return new WaitForSeconds(time);
        _coSkillCoolTime = null;
    }

    protected override void CheckUpdateFlag()
    {
        if (_updated)
        {
            C_Move movePacket = new C_Move();
            movePacket.PosInfo = PosInfo;
            //Debug.Log($"{transform.name} : C_MOVE({movePacket.PosInfo.PosX}, {movePacket.PosInfo.PosY})");
            Managers.Network.Send(movePacket);
            _updated = false;
        }
    }

    // 추가 스텟 계산
    public void RefreshAdditionalStat()
    {
        // 처음부터 계산하는게 속편함
        WeaponDamage = 0;
        ArmorDefence = 0;

        foreach (Item item in Managers.Inven.Items.Values)
        {
            if (item.Equipped == false) continue;

            switch (item.ItemType)
            {
                case ItemType.Weapon:
                    WeaponDamage += ((Weapon)item).Damage;
                    break;
                case ItemType.Armor:
                    ArmorDefence += ((Armor)item).Defence;
                    break;
            }
        }
    }
}
