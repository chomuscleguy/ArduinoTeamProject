using System.Collections.Generic;
using System;

[Serializable]
public class DeviceDataListWrapper
{
    public List<DeviceData> devices;

    public Dictionary<string, DeviceData> ToDictionary()
    {
        Dictionary<string, DeviceData> dict = new();
        foreach (var device in devices)
        {
            dict[device.Mac] = device;
        }
        return dict;
    }
}
