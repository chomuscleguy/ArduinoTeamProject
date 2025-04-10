using System.Collections.Generic;
using UnityEngine;

public class PairedData : MonoBehaviour
{
    public void PrintAllDevices()
    {
        foreach (var kv in DataManager.Instance.deviceData)
        {
            var d = kv.Value;
        }
    }

    public void RenameDevice(string mac, string newName)
    {
        if (DataManager.Instance.deviceData.TryGetValue(mac, out var data))
        {
            data.name = newName;
            DataManager.Instance.SaveData();
        }
    }

    public void ChangeIndex(string mac, int newIndex)
    {
        if (DataManager.Instance.deviceData.TryGetValue(mac, out var data))
        {
            data.index = newIndex;
            DataManager.Instance.SaveData();
        }
    }

    public void DeleteDevice(string mac)
    {
        if (DataManager.Instance.deviceData.Remove(mac))
        {
            ReorderIndices();
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
