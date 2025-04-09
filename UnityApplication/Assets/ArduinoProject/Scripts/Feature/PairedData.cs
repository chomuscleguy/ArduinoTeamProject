using System.Collections.Generic;
using UnityEngine;

public class PairedData : MonoBehaviour
{
    public void PrintAllDevices()
    {
        foreach (var kv in DataManager.Instance.deviceData)
        {
            var d = kv.Value;
            Debug.Log($"[{d.index}] {d.Mac} - {d.name} / Auto: {d.autoPaired}");
        }
    }

    public void RenameDevice(string mac, string newName)
    {
        if (DataManager.Instance.deviceData.TryGetValue(mac, out var data))
        {
            data.name = newName;
            Debug.Log($"✅ {mac} 이름 변경: {newName}");
            DataManager.Instance.SaveData();
        }
    }

    public void ChangeIndex(string mac, int newIndex)
    {
        if (DataManager.Instance.deviceData.TryGetValue(mac, out var data))
        {
            data.index = newIndex;
            Debug.Log($"📌 {mac} index 변경: {newIndex}");
            DataManager.Instance.SaveData();
        }
    }

    public void DeleteDevice(string mac)
    {
        if (DataManager.Instance.deviceData.Remove(mac))
        {
            Debug.Log($"❌ {mac} 기기 삭제됨");
            ReorderIndices(); // 선택: index 재정렬
            DataManager.Instance.SaveData();
        }
    }

    private void ReorderIndices()
    {
        var list = new List<DeviceData>(DataManager.Instance.deviceData.Values);
        list.Sort((a, b) => a.index.CompareTo(b.index));
        for (int i = 0; i < list.Count; i++)
        {
            list[i].index = i + 1;
        }
    }

    
}
