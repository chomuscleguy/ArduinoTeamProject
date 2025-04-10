using System;
using System.Collections.Generic;
using System.IO;
using System.Net.Mail;
using TMPro;
using UnityEngine;
using UnityEngine.Android;
using UnityEngine.UI;

public class BluetoothManager : MonoBehaviour
{
    public event Action<bool> OnConnectionStatusChanged;

    private Dictionary<string, bool> connectedDevices = new();
    private string currentConnectingMac = null;
    private string ConnetedMac = null;
    private bool _isConnected;

    public bool isConnected
    {
        get => _isConnected;
        set
        {
            if (_isConnected != value)
            {
                _isConnected = value;
                OnConnectionStatusChanged?.Invoke(value);
            }
        }
    }

    private static AndroidJavaClass unity3dbluetoothplugin;
    private static AndroidJavaObject BluetoothConnector;

    private static BluetoothManager _instance;

    public static BluetoothManager Instance
    {
        get
        {
            if (_instance == null)
            {
                _instance = FindObjectOfType<BluetoothManager>();
                if (_instance == null)
                {
                    GameObject obj = new GameObject(nameof(BluetoothManager));
                    _instance = obj.AddComponent<BluetoothManager>();
                }
            }
            return _instance;
        }
    }

    public TMP_InputField sendData;
    public Text receivedData;
    public GameObject scanedListContainer;
    public GameObject pairedListContainer;
    public GameObject deviceMACText;
    public GameObject addDevice;

    public GameObject scanedDevice;
    public GameObject pairedDevice;


    void Start()
    {
        InitBluetooth();
        GetPairedDevices();
    }

    public void InitBluetooth()
    {
        if (Application.platform != RuntimePlatform.Android) return;

        string[] permissions = {
            Permission.CoarseLocation,
            Permission.FineLocation,
            "android.permission.BLUETOOTH_ADMIN",
            "android.permission.BLUETOOTH",
            "android.permission.BLUETOOTH_SCAN",
            "android.permission.BLUETOOTH_ADVERTISE",
            "android.permission.BLUETOOTH_CONNECT"
        };

        List<string> toRequest = new();
        foreach (string perm in permissions)
        {
            if (!Permission.HasUserAuthorizedPermission(perm))
                toRequest.Add(perm);
        }

        if (toRequest.Count > 0)
            Permission.RequestUserPermissions(toRequest.ToArray());

        unity3dbluetoothplugin = new AndroidJavaClass("com.example.unity3dbluetoothplugin.BluetoothConnector");
        BluetoothConnector = unity3dbluetoothplugin.CallStatic<AndroidJavaObject>("getInstance");
    }

    public void StartScanDevices()
    {
        if (Application.platform != RuntimePlatform.Android) return;

        foreach (Transform child in scanedListContainer.transform)
        {
            Destroy(child.gameObject);
        }

        BluetoothConnector.CallStatic("StartScanDevices");
    }

    public void StopScanDevices()
    {
        if (Application.platform != RuntimePlatform.Android) return;

        BluetoothConnector.CallStatic("StopScanDevices");
    }

    public void NewDeviceFound(string data)
    {
        int macLength = 18;
        GameObject newDevice = Instantiate(scanedDevice, scanedListContainer.transform);

        string BLEName = data.Substring(macLength).Trim().ToUpper();

        if (BLEName == "null")
        {
            Destroy(newDevice);
            return;
        }

        newDevice.GetComponentInChildren<TextMeshProUGUI>().text = BLEName;
        newDevice.name = data;
    }

    public void StartConnection(string mac)
    {
        if (Application.platform != RuntimePlatform.Android) return;

        currentConnectingMac = mac;
        BluetoothConnector.CallStatic("StartConnection", mac);
    }

    public void StopConnection()
    {
        if (Application.platform != RuntimePlatform.Android) return;

        if (isConnected)
            BluetoothConnector.CallStatic("StopConnection");
    }

    public void ConnectionStatus(string status)
    {
        Toast("Connection Status: " + status);
        isConnected = status == "connected";

        if (isConnected)
        {
            SavePairedDevice(currentConnectingMac);
            ConnetedMac = currentConnectingMac; 
        }

        isConnected = false;
    }

    private void SavePairedDevice(string mac)
    {
        if (DataManager.Instance.deviceData.ContainsKey(mac))
        {
            Debug.Log($"🔁 이미 저장된 기기: {mac}");
            return;
        }

        string bleName = "UNKNOWN_DEVICE";

        foreach (Transform child in scanedListContainer.transform)
        {
            if (child.name.StartsWith(mac))
            {
                var tmp = child.GetComponentInChildren<TextMeshProUGUI>();
                if (tmp != null)
                {
                    bleName = tmp.text;
                    break;
                }
            }
        }

        var deviceData = new DeviceData
        {
            Mac = mac,
            index = DataManager.Instance.deviceData.Count + 1,
            name = bleName,
            description = "수동 연결됨",
            autoPaired = false
        };

        DataManager.Instance.deviceData[mac] = deviceData;
        DataManager.Instance.SaveData();
    }

    public void GetPairedDevices()
    {
        if (Application.platform != RuntimePlatform.Android) return;

        Toast("📡 페어링된 기기 스캔 중...");
        string[] data = BluetoothConnector.CallStatic<string[]>("GetPairedDevices");

        for (int i = pairedListContainer.transform.childCount - 1; i >= 0; i--)
        {
            Transform child = pairedListContainer.transform.GetChild(i);
            if (!child.name.Contains("AddDevice"))
                Destroy(child.gameObject);
        }

        foreach (var d in data)
        {
            int macLength = 18;
            string mac = d.Substring(0, macLength).Trim();
            string BLEName = d.Substring(macLength).Trim().ToUpper();

            if (BLEName == "null" || string.IsNullOrEmpty(BLEName))
                BLEName = "알 수 없는 기기";

            if (!DataManager.Instance.deviceData.ContainsKey(mac))
            {
                var newDeviceData = new DeviceData
                {
                    Mac = mac,
                    index = DataManager.Instance.deviceData.Count + 1,
                    name = BLEName,
                    description = "페어링됨",
                    autoPaired = false,
                };

                DataManager.Instance.deviceData[mac] = newDeviceData;
            }
        }
        DataManager.Instance.SaveData();

        InitPairedDevice();

        //var sortedDevices = new List<DeviceData>(DataManager.Instance.deviceData.Values);
        //sortedDevices.Sort((a, b) => a.index.CompareTo(b.index));

        //foreach (var device in sortedDevices)
        //{
        //    GameObject newDevice = Instantiate(pairedDevice, pairedListContainer.transform);
        //    newDevice.name = device.Mac;
        //    newDevice.GetComponentInChildren<TextMeshProUGUI>().text = string.IsNullOrEmpty(device.name) ? "알 수 없는 기기" : device.name;
        //}

        addDevice.transform.SetAsLastSibling();
    }

    public void ReadData(string data)
    {
        receivedData.text = data;
    }

    public void WriteData(string txt)
    {
        if (Application.platform != RuntimePlatform.Android) return;

        BluetoothConnector.CallStatic("WriteData", txt);
    }

    public void testsss( )
    {
        string textToSend = sendData.text;

        BluetoothConnector.CallStatic("WriteData", sendData.text);
    }

    public void Toast(string message)
    {
        if (Application.platform != RuntimePlatform.Android) return;

        BluetoothConnector.CallStatic("Toast", message);
    }


    void OnApplicationQuit()
    {
        if (isConnected)
        {
            BluetoothConnector.CallStatic("StopConnection");
        }
    }

    private void InitPairedDevice()
    {
        DataManager.Instance.LoadData();

        var sortedDevices = new List<DeviceData>(DataManager.Instance.deviceData.Values);
        sortedDevices.Sort((a, b) => a.index.CompareTo(b.index));

        foreach (var device in sortedDevices)
        {
            Transform existing = pairedListContainer.transform.Find(device.Mac);
            if (existing != null)
            {
                var text = existing.GetComponentInChildren<TextMeshProUGUI>();
                if (text != null)
                {
                    text.text = string.IsNullOrEmpty(device.name) ? "알 수 없는 기기" : device.name;
                }
            }

            GameObject newDevice = Instantiate(pairedDevice, pairedListContainer.transform);
            newDevice.name = device.Mac;

            newDevice.GetComponentInChildren<TextMeshProUGUI>().text =
                string.IsNullOrEmpty(device.name) ? "알 수 없는 기기" : device.name;

            Toast(device.name);

            newDevice.GetComponentInChildren<CircularUI>().gameObject.SetActive(true);
        }

    }
}
