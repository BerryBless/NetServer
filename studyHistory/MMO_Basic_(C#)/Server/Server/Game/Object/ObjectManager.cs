using System;
using System.Collections.Generic;
using System.Text;
using Google.Protobuf.Protocol;

namespace Server.Game
{
    public class ObjectManager
    {
        public static ObjectManager Instance { get; } = new ObjectManager();

        object _lock = new object();


        Dictionary<int, GameObject> _objects = new Dictionary<int, GameObject>();

        // 오브젝트 아이디 관리
        // int :: [안씀(1)][TYPE(7)][ID(24)]
        int _counter = 1;

        // Objtect 딕셔너리에 넣기
        public T Add<T>() where T : GameObject, new ()  
        {
            T gameObject = new T();
            lock (_lock)
            {
                // ID발급
                gameObject.Id = GenerateId(gameObject.ObjectType);

                if(gameObject.ObjectType == GameObjectType.Player)
                {
                    _objects.Add(gameObject.Id, gameObject as Player);
                }
            }
            return gameObject;
        }
        // Type에 따른 Id 반환
        int GenerateId(GameObjectType type)
        {
            lock (_lock)
            {
                return ((int)type << 24) | (_counter++);
            }
        }
        // ID에 따른 타입 반환
        public static GameObjectType GetObjectTypeById(int id)
        {
            int type = (id >> 24) & 0x7F;
            return (GameObjectType)type;
        }

        // 딕셔너리에서 지우기
        public bool Remove(int objectId)
        {
            GameObjectType objectType = GetObjectTypeById(objectId);
            lock (_lock)
            {
                if(objectType == GameObjectType.Player)
                    return _objects.Remove(objectId);

            }
            return false;
        }

        // 딕셔너리에서 찾기
        public GameObject Find(int objectId)
        {
            GameObjectType objectType = GetObjectTypeById(objectId);
            lock (_lock)
            {
                if (objectType == GameObjectType.Player)
                {
                    GameObject obj = null;
                    if (_objects.TryGetValue(objectId, out obj))
                        return obj;
                }
            }
                return null;
        }
    }
}
