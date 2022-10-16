using Google.Protobuf.Protocol;
using Server.Data;
using Server.DB;
using System;
using System.Collections.Generic;
using System.Text;

namespace Server.Game
{
    // server에서! 들고있는! 아이템!
    public class Item
    {
        public ItemInfo info = new ItemInfo();
        public int ItemDbId { get { return info.ItemDbId; } set { info.ItemDbId = value; } }        // Db에서 들고있는 아이템
        public int TemplateId { get { return info.TemplateId; } set { info.TemplateId = value; } }  // 데이터시트 ID
        public int Count { get { return info.Count; } set { info.Count = value; } }                 // 플레이어가 들고있는 이 아이템개수
        public int Slot { get { return info.Slot; } set { info.Slot = value; } }                    // 플레이어가 들고있는 이 아이템개수
        public bool Equipped { get { return info.Equipped; } set { info.Equipped = value; } }       // 장착된 아이템이냐


        public ItemType ItemType { get; private set; }  // 아이템타입
        public bool Stackable { get; protected set; }   //  아이템 겹처지냐 캐싱 (타입으로 매번 알아내기 귀찮음)

        public Item(ItemType itemType)
        {
            ItemType = itemType;
        }

        public static Item MakeItem(ItemDb itemDb)
        {
            Item item = null;

            ItemData itemData = null;
            DataManager.ItemDict.TryGetValue(itemDb.TemplateId, out itemData);

            if (itemData == null) return null;

            switch (itemData.itemType)
            {
                case ItemType.Weapon:
                    item = new Weapon(itemDb.TemplateId);
                    break;
                case ItemType.Armor:
                    item = new Armor(itemDb.TemplateId);
                    break;
                case ItemType.Consumable:
                    item = new Consumable(itemDb.TemplateId);
                    break;
            }
            if (item != null)
            {
                item.ItemDbId = itemDb.ItemDbId;
                item.Count = itemDb.Count;
                item.Slot = itemDb.Slot;
                item.Equipped = itemDb.Equipped;
            }
            return item;
        }
    }
    // 무기
    public class Weapon : Item
    {
        public WeaponType WeaponType { get; private set; }
        public int Damage { get; private set; }
        public Weapon(int templateId) : base(ItemType.Weapon)
        {
            Init(templateId);
        }

        void Init(int templateId)
        {
            ItemData itemData = null;
            DataManager.ItemDict.TryGetValue(templateId, out itemData);
            if (itemData.itemType != ItemType.Weapon) // 심각한 문제 갑옷으로적 때리기? 어? 좋은데?
                return;

            WeaponData data = itemData as WeaponData;
            {
                TemplateId = data.id;
                Count = 1;
                WeaponType = data.weaponType;
                Damage = data.damage;
                Stackable = false;
            }
        }
    }
    // 방어구
    public class Armor : Item
    {
        public ArmorType ArmorType { get; private set; }
        public int Defence { get; private set; }
        public Armor(int templateId) : base(ItemType.Armor)
        {
            Init(templateId);
        }

        void Init(int templateId)
        {
            ItemData itemData = null;
            DataManager.ItemDict.TryGetValue(templateId, out itemData);
            if (itemData.itemType != ItemType.Armor)
                return;

            ArmorData data = itemData as ArmorData;
            {
                TemplateId = data.id;
                Count = 1;
                ArmorType = data.armorType;
                Defence = data.defence;
                Stackable = false;
            }
        }
    }
    // 소모품
    public class Consumable : Item
    {
        public ConsumableType ConsumableType { get; private set; }
        public int MaxCount { get; private set; }
        public Consumable(int templateId) : base(ItemType.Consumable)
        {
            Init(templateId);
        }

        void Init(int templateId)
        {
            ItemData itemData = null;
            DataManager.ItemDict.TryGetValue(templateId, out itemData);
            if (itemData.itemType != ItemType.Consumable)
                return;

            ConsumableData data = itemData as ConsumableData;
            {
                TemplateId = data.id;
                Count = 1;
                MaxCount = data.maxCount;
                ConsumableType = data.consumableType;
                Stackable = (data.maxCount > 1);
            }
        }
    }
}
