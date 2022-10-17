using System.Collections;
using System.Collections.Generic;
using UnityEngine;


// 서버정보
public class ServerInfo
{
    // 서버이름
    public string Name;
    // 서버주소
    public string IpAddress;
    public int Port;
    // 혼잡도, 높을 수 록 혼잡
    public int BusyScore; 
}

// 클라에서 서버
#region CLIENT => SERVER
// 계정생성 시도 
public class CreateAccountPacketReq
{
    public string AccountName;
    public string Password;
}

// login
public class LoginAccountPacketReq
{
    public string AccountName;
    public string Password;
}
#endregion


// 서버에서 클라
#region SERVER => CLIENT
// 계정생성 성공여부
public class CreateAccountPacketRes
{
    public bool CreateOk;
}

// login
public class LoginAccountPacketRes
{
    public bool LoginOk;
    public int AccountId;
    public int Token;
    public List<ServerInfo> ServerList = new List<ServerInfo>();
}

#endregion

