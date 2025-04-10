using System.Collections.Generic;
using TMPro;
using UnityEngine;

public class SidebarSet : MonoBehaviour
{
    public DB_Device device;
    public GameObject pairedDevice;
    public Transform pairedListContainer;

    public void InitData()
    {
        DataManager.Instance.LoadData();

        var sortedDevices = new List<DeviceData>(DataManager.Instance.deviceData.Values);
        sortedDevices.Sort((a, b) => a.index.CompareTo(b.index));

        BluetoothManager.Instance.Toast($"[SidebarSet] 불러온 기기 수: {sortedDevices.Count}");

        foreach (var device in sortedDevices)
        {
            Transform existing = pairedListContainer.Find(device.Mac);
            if (existing != null)
            {
                var text = existing.GetComponentInChildren<TextMeshProUGUI>();
                if (text != null)
                {
                    text.text = string.IsNullOrEmpty(device.name) ? "알 수 없는 기기" : device.name;
                }

                var renamer = existing.GetComponent<Rename>();
                if (renamer != null)
                {
                    renamer.SetMacAddress(device.Mac);
                }

                continue;
            }

            GameObject newDevice = Instantiate(pairedDevice, pairedListContainer);
            newDevice.name = device.Mac;

            newDevice.GetComponentInChildren<TextMeshProUGUI>().text =
                string.IsNullOrEmpty(device.name) ? "알 수 없는 기기" : device.name;

            var renamerNew = newDevice.GetComponent<Rename>();

            if (renamerNew != null)
            {
                renamerNew.SetMacAddress(device.Mac);
            }
        }
    }
}
