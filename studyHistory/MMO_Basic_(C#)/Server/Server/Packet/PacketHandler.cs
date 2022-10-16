using Google.Protobuf;
using Google.Protobuf.Protocol;
using Server;
using Server.DB;
using Server.Game;
using ServerCore;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

class PacketHandler
{
    public static object Room { get; private set; }
    public static GameRoom GameRoom { get; private set; }

    public static void C_MoveHandler(PacketSession session, IMessage packet)
    {
        // 클라에서 뭔가 움직임!
        C_Move movePacket = packet as C_Move;
        ClientSession clientSession = session as ClientSession;

        //Console.WriteLine($"{clientSession.MyPlayer.Info.ObjectId} : C_MOVE ({movePacket.PosInfo.PosX}, {movePacket.PosInfo.PosY}) Dir : {clientSession.MyPlayer.Info.PosInfo.MoveDir}");

        Player player = clientSession.MyPlayer;
        if (player == null) return;

        GameRoom room = clientSession.MyPlayer.Room;
        if (room == null) return;

        //room.HandleMove(player, movePacket);
        room.Push(room.HandleMove, player, movePacket);
    }

    public static void C_SkillHandler(PacketSession session, IMessage packet)
    {
        // 스킬 사용 시도
        C_Skill skillPacket = packet as C_Skill;
        ClientSession clientSession = session as ClientSession;

        //Console.WriteLine($"{clientSession.MyPlayer.Info.ObjectId} : C_SKILL ({skillPacket.Info.SkillId})");

        Player player = clientSession.MyPlayer;
        if (player == null) return;

        GameRoom room = clientSession.MyPlayer.Room;
        if (room == null) return;
        // 스킬처리
        //room.HandleSkill(player, skillPacket);
        room.Push(room.HandleSkill, player, skillPacket);

    }

    public static void C_LoginHandler(PacketSession session, IMessage packet)
    {
        // 로그인 시도
        C_Login loginPacket = packet as C_Login;
        ClientSession clientSession = session as ClientSession;

        clientSession.HandleLogin(loginPacket);
    }

    public static void C_EnterGameHandler(PacketSession session, IMessage packet)
    {
        // 게임 접속 시도
        C_EnterGame enterGamePacket = packet as C_EnterGame;
        ClientSession clientSession = session as ClientSession;

        clientSession.HandleEnterGame(enterGamePacket);
    }

    public static void C_CreatePlayerHandler(PacketSession session, IMessage packet)
    {
        // 캐릭터 생성 시도
        C_CreatePlayer createPlayerPacket = packet as C_CreatePlayer;
        ClientSession clientSession = session as ClientSession;

        clientSession.HandleCreatePlayer(createPlayerPacket);
    }
    public static void C_EquipItemHandler(PacketSession session, IMessage packet)
    {
        // 클라 : 이 무기를 장착/해지 하고싶어!
        C_EquipItem equipItemPacket = packet as C_EquipItem;
        ClientSession clientSession = session as ClientSession;

        // 게임룸
        Player player = clientSession.MyPlayer;
        if (player == null) return;

        GameRoom room = clientSession.MyPlayer.Room;
        if (room == null) return;
        // 스킬처리
        //room.HandleSkill(player, skillPacket);
        room.Push(room.HandleEquipItem, player, equipItemPacket);

        Console.WriteLine($"equipItem ({equipItemPacket.ItemDbId})");
    }
    public static void C_PongHandler(PacketSession session, IMessage packet)
    {
        ClientSession clientSession = session as ClientSession;
        clientSession.HandlePong();
    }

}
