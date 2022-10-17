using Google.Protobuf;
using Google.Protobuf.Protocol;
using ServerCore;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

class PacketHandler
{
    public static void S_EnterGameHandler(PacketSession session, IMessage packet)// "이몸등장"
    {
        S_EnterGame enterGamePacket = packet as S_EnterGame;
        Managers.Object.Add(enterGamePacket.Player, myPlayer: true);
    }
    public static void S_LeaveGameHandler(PacketSession session, IMessage packet)// "이몸퇴장"
    {
        S_LeaveGame leaveGamePacket = packet as S_LeaveGame;

        Managers.Object.Clear();
    }
    public static void S_SpawnHandler(PacketSession session, IMessage packet)// ㅎㅇ
    {
        S_Spawn spawnPacket = packet as S_Spawn;

        foreach (ObjectInfo obj in spawnPacket.Objects)
        {
            //Debug.Log($"S_MoveHandler ::\n\t{obj.ObjectId} :: CellPos ({obj.PosInfo.PosX},{obj.PosInfo.PosY}) | Dir : {obj.PosInfo.MoveDir}");
            Managers.Object.Add(obj, myPlayer: false);
        }
    }
    public static void S_DespawnHandler(PacketSession session, IMessage packet)// ㅂㅇ
    {
        S_Despawn despawnPacket = packet as S_Despawn;

        foreach (int obj in despawnPacket.ObjectIds)
        {
            Managers.Object.Remove(obj);
        }
    }
    public static void S_MoveHandler(PacketSession session, IMessage packet)// 무언가 이동
    {
        S_Move movePacket = packet as S_Move;

        GameObject go = Managers.Object.Find(movePacket.ObjectId);
        if (go == null) return;


        if (movePacket.ObjectId == Managers.Object.MyPlayer.Id) return;

        BaseController bc = go.GetComponent<BaseController>();
        if (bc == null) return;

        //Debug.Log($"S_MoveHandler ::\n\t{cc.gameObject.name} :: CellPos ({cc.PosInfo.PosX},{cc.PosInfo.PosY}) | Dir : {cc.PosInfo.MoveDir}");
        bc.PosInfo = movePacket.PosInfo;
    }

    public static void S_SkillHandler(PacketSession session, IMessage packet) // 무언가 스킬 사용
    {
        S_Skill skillPacket = packet as S_Skill;

        GameObject go = Managers.Object.Find(skillPacket.ObjectId);
        if (go == null) return;

        CreatureController cc = go.GetComponent<CreatureController>();
        if (cc != null)
        {
            //Debug.Log($"skillPacket.Info.SkillId({skillPacket.Info.SkillId})");
            cc.UseSkill(skillPacket.Info.SkillId);
        }

    }
    public static void S_ChangeHpHandler(PacketSession session, IMessage packet) // 누군가의 HP 가 줄어듦
    {
        S_ChangeHp changePacket = packet as S_ChangeHp;

        GameObject go = Managers.Object.Find(changePacket.ObjectId);
        if (go == null) return;

        CreatureController cc = go.GetComponent<CreatureController>();
        if (cc != null)
        {
            cc.Hp = changePacket.Hp;
        }

    }
    public static void S_DieHandler(PacketSession session, IMessage packet) // 무언가 소멸됨
    {
        S_Die diePacket = packet as S_Die;

        GameObject go = Managers.Object.Find(diePacket.ObjectId);
        if (go == null) return;

        CreatureController cc = go.GetComponent<CreatureController>();
        if (cc != null)
        {
            // TODO 죽음 이펙트
            cc.Hp = 0;
            cc.OnDead();
        }

    }
    public static void S_ConnectedHandler(PacketSession session, IMessage packet)// 연결
    {
        // S_Connected connectedPacket = packet as S_Connected;
        // 연결중입니다

        Debug.Log("S_ConnectedHandler");
        C_Login loginPacket = new C_Login();

        // 클라이언트 겹치지 않는 아이디를 만들려고
        string path = Application.dataPath;
        loginPacket.UniqueId = path.GetHashCode().ToString();

        Managers.Network.Send(loginPacket);

    }

    public static void S_LoginHandler(PacketSession session, IMessage packet)// 로그인 + 캐릭터 목록
    {
        S_Login loginPacket = packet as S_Login;
        // TODO 로그인 성공/실패

        Debug.Log($"loginOK({loginPacket.LoginOk})");

        // TODO : 로비에서 캐릭터 목록 보여주고, 선택
        if (loginPacket.Players == null || loginPacket.Players.Count == 0)
        {
            C_CreatePlayer createPacket = new C_CreatePlayer();
            createPacket.Name = $"Player_{Random.Range(0, 10000).ToString("0000")}";
            Managers.Network.Send(createPacket);
        }
        else
        {
            // 무조건 첫번쨰 캐릭터 로그인
            LobbyPlayerInfo info = loginPacket.Players[0];

            C_EnterGame enterGamePacket = new C_EnterGame();
            enterGamePacket.Name = info.Name;
            Managers.Network.Send(enterGamePacket);
        }
    }
    public static void S_CreatePlayerHandler(PacketSession session, IMessage packet)// 플레이어 생성
    {
        S_CreatePlayer createOkPacket = packet as S_CreatePlayer;
        Debug.Log("CreatePlayer");
        if (createOkPacket.Player == null)
        {
            // 플레이어 만들기 실패
            C_CreatePlayer createPacket = new C_CreatePlayer();
            createPacket.Name = $"Player_{Random.Range(0, 10000).ToString("0000")}";
            Managers.Network.Send(createPacket);
        }
        else
        {
            C_EnterGame enterGamePacket = new C_EnterGame();
            enterGamePacket.Name = createOkPacket.Player.Name;
            Managers.Network.Send(enterGamePacket);
        }
    }
    public static void S_ItemListHandler(PacketSession session, IMessage packet)// 게임 처음 시작할떄 인벤토리 에 넣기
    {
        S_ItemList itemList = packet as S_ItemList;

        // 아이템정보 메모리에 들고있기
        foreach (ItemInfo itemInfo in itemList.Items)
        {
            Item item = Item.MakeItem(itemInfo);
            Managers.Inven.Add(item);
        }
        if (Managers.Object.MyPlayer != null)
            Managers.Object.MyPlayer.RefreshAdditionalStat();
    }

    public static void S_AddItemHandler(PacketSession session, IMessage packet)// 인게임중 아이템 획득
    {
        S_AddItem itemList = packet as S_AddItem;

        // 아이템정보 메모리에 들고있기
        foreach (ItemInfo itemInfo in itemList.Items)
        {
            Item item = Item.MakeItem(itemInfo);
            Managers.Inven.Add(item);
        }

        Debug.Log("아이템 획득!");
        // 인벤토리 새로고침
        UI_GameScene gameSceneUI = Managers.UI.SceneUI as UI_GameScene;
        gameSceneUI.InvenUI.RefreshUI();
        gameSceneUI.StatUI.RefreshUI();
        // 변경스텟 적용
        if (Managers.Object.MyPlayer != null)
            Managers.Object.MyPlayer.RefreshAdditionalStat();
    }
    public static void S_EquipItemHandler(PacketSession session, IMessage packet)// 아이템 장착 / 탈착 
    {
        S_EquipItem equipItemOk = packet as S_EquipItem;

        Item item = Managers.Inven.Get(equipItemOk.ItemDbId);
        if (item == null)
            return;

        item.Equipped = equipItemOk.Equipped;
        Debug.Log("아이템 착용 변경");
        // 인벤토리 새로고침
        UI_GameScene gameSceneUI = Managers.UI.SceneUI as UI_GameScene;
        gameSceneUI.InvenUI.RefreshUI();
        gameSceneUI.StatUI.RefreshUI();
        // 변경스텟 적용
        if (Managers.Object.MyPlayer != null)
            Managers.Object.MyPlayer.RefreshAdditionalStat();
    }
    public static void S_ChangeStatHandler(PacketSession session, IMessage packet)// 캐릭터 스텟 바꾸기
    {
        S_ChangeStat changeStatPacket = packet as S_ChangeStat;
    }
    
    public static void S_PingHandler(PacketSession session, IMessage packet)// 서버랑 핑퐁~ (계속 연결중인가 확인)
    {
        C_Pong pongPacket = new C_Pong();
        Debug.Log("[Server] PingCheck");
        Managers.Network.Send(pongPacket);
    }

    public static void S_ChangeMapHandler(PacketSession session, IMessage packet)// 맵로딩
    {
        S_ChangeMap changeMapPacket = packet as S_ChangeMap;
        // TODO 맵바꾸기
        Managers.Map.LoadMap(changeMapPacket.MapId);
    }

}
