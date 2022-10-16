using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class UI_GameScene : UI_Scene
{
    public UI_Stat StatUI { get; private set; }
    public UI_Inventory InvenUI { get; private set; }

    public override void Init()
    {
        base.Init();

        InvenUI = GetComponentInChildren<UI_Inventory>();
        StatUI = GetComponentInChildren<UI_Stat>();

        InvenUI.gameObject.SetActive(false);
        StatUI.gameObject.SetActive(false);
    }
}
