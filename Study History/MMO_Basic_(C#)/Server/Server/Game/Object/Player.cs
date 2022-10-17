using Google.Protobuf.Protocol;
using Microsoft.EntityFrameworkCore;
using Server.DB;
using System;
using System.Collections.Generic;
using System.Text;

namespace Server.Game
{
    public class Player : GameObject
    {
        public int PlayerDbId { get; set; }
        public ClientSession Session { get; set; }
        public Inventory Inven { get; set; } = new Inventory();
        public VisionCube Vision { get; private set; } 

        public int WeaponDamage { get; private set; }
        public int ArmorDefence { get; private set; }

        public override int TotalAttack { get { return Stat.Attack + WeaponDamage; } }
        public override int TotalDefence { get { return ArmorDefence; } }

        public Player()
        {
            ObjectType = GameObjectType.Player;
            Vision = new VisionCube(this);
        }

        public override void OnDamaged(GameObject attecker, int damage)
        {
            base.OnDamaged(attecker, damage);
            //Console.WriteLine($"player OnDamaged :: {attecker.Stat.Attack} from ID({attecker.Id})");
        }
        public override void OnDead(GameObject attecker)
        {
            base.OnDead(attecker);
            // 이 다음 업데이트할 비전큐브는 필요없다
            Vision.Clear();
            Vision._job.Cancel = true; 
        }

        public void OnLeaveGame()
        {
            DbTransaction.SavePlayerStatus_AllInOne(this, Room);
            // 이 다음 업데이트할 비전큐브는 필요없다
            Vision._job.Cancel = true;
        }

        public void HandleEquipItem(C_EquipItem equipPacket)
        {
            // 아이템 있나 체크
            Item item = this.Inven.Get(equipPacket.ItemDbId);
            if (item == null) return;

            if (item.ItemType == ItemType.Consumable)
                return;//소모품은 장착아님

            // 장착 요청이면 겹치는 부위 해제
            if (equipPacket.Equipped)
            {
                Item unequipItem = null;

                // 무기는 하나만
                if (item.ItemType == ItemType.Weapon)
                {
                    unequipItem = this.Inven.Find(
                        i => i.Equipped && i.ItemType == ItemType.Weapon);
                }
                // 방어구는 비교
                else if (item.ItemType == ItemType.Armor)
                {
                    ArmorType armorType = ((Armor)item).ArmorType;
                    unequipItem = this.Inven.Find(
                        i => i.Equipped && i.ItemType == ItemType.Armor && ((Armor)i).ArmorType == armorType);
                }
                // 겹치는 아이템 벗기
                if (unequipItem != null)
                {
                    // 메모리 선적용
                    unequipItem.Equipped = false;

                    // DB에 Noti
                    DbTransaction.EquipItemNoti(this, unequipItem);

                    // 클라에 통보
                    S_EquipItem equipOkItem = new S_EquipItem();
                    equipOkItem.ItemDbId = unequipItem.ItemDbId;
                    equipOkItem.Equipped = unequipItem.Equipped;
                    this.Session.Send(equipOkItem);
                }
            }
            // 요청사항 적용
            {
                // 메모리 선적용
                item.Equipped = equipPacket.Equipped;

                // DB에 Noti
                DbTransaction.EquipItemNoti(this, item);

                // 클라에 통보
                S_EquipItem equipOkItem = new S_EquipItem();
                equipOkItem.ItemDbId = equipPacket.ItemDbId;
                equipOkItem.Equipped = equipPacket.Equipped;
                this.Session.Send(equipOkItem);
            }
            RefreshAdditionalStat();
        }

        // 추가 스텟 계산
        public void RefreshAdditionalStat()
        {
            // 처음부터 계산하는게 속편함
            WeaponDamage = 0;
            ArmorDefence = 0;

            foreach (Item item in Inven.Items.Values)
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
}
