using System;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.Android;
using UnityEngine.UI;

public class BluetoothManager : MonoBehaviour
{
    #region Event
    public static event Action<bool> OnConnectionStatusChanged;
    #endregion

    private Dictionary<string, bool> connectedDevices = new Dictionary<string, bool>();

    public Text deviceAdd;
    public Text receivedData;
    public GameObject scanedListContainer;
    public GameObject pairedListContainer;
    public GameObject deviceMACText;
    public GameObject addDevice;

    public GameObject scanedDevice;
    public GameObject pairedDevice;

    private string currentConnectingMac = null;

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
                    GameObject singletonObject = new GameObject(typeof(BluetoothManager).ToString());
                    _instance = singletonObject.AddComponent<BluetoothManager>();
                }
            }
            return _instance;
        }
    }

    void Start()
    {
        InitBluetooth();
        GetPairedDevices();
    }

    public void InitBluetooth()
    {
        if (Application.platform != RuntimePlatform.Android)
            return;

        if (!Permission.HasUserAuthorizedPermission(Permission.CoarseLocation)
            || !Permission.HasUserAuthorizedPermission(Permission.FineLocation)
            || !Permission.HasUserAuthorizedPermission("android.permission.BLUETOOTH_ADMIN")
            || !Permission.HasUserAuthorizedPermission("android.permission.BLUETOOTH")
            || !Permission.HasUserAuthorizedPermission("android.permission.BLUETOOTH_SCAN")
            || !Permission.HasUserAuthorizedPermission("android.permission.BLUETOOTH_ADVERTISE")
            || !Permission.HasUserAuthorizedPermission("android.permission.BLUETOOTH_CONNECT"))
        {
            Permission.RequestUserPermissions(new string[] {
                            Permission.CoarseLocation,
                            Permission.FineLocation,
                            "android.permission.BLUETOOTH_ADMIN",
                            "android.permission.BLUETOOTH",
                            "android.permission.BLUETOOTH_SCAN",
                            "android.permission.BLUETOOTH_ADVERTISE",
                            "android.permission.BLUETOOTH_CONNECT"
                        });
        }

        unity3dbluetoothplugin = new AndroidJavaClass("com.example.unity3dbluetoothplugin.BluetoothConnector");
        BluetoothConnector = unity3dbluetoothplugin.CallStatic<AndroidJavaObject>("getInstance");
    }
    public void StartScanDevices()
    {
        if (Application.platform != RuntimePlatform.Android)
            return;

        foreach (Transform child in scanedListContainer.transform)
        {
            Destroy(child.gameObject);
        }

        BluetoothConnector.CallStatic("StartScanDevices");
    }

    public void StopScanDevices()
    {
        if (Application.platform != RuntimePlatform.Android)
            return;

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

    //public void StartConnection(string text)
    //{
    //    if (Application.platform != RuntimePlatform.Android)
    //        return;

    //    string deviceMacAddress = text;

    //    BluetoothConnector.CallStatic("StartConnection", deviceMacAddress);
    //}

    public void StartConnection(string text)
    {
        if (Application.platform != RuntimePlatform.Android)
            return;

        string deviceMacAddress = text;
        currentConnectingMac = deviceMacAddress;

        BluetoothConnector.CallStatic("StartConnection", deviceMacAddress);
    }


    public void StopConnection()
    {
        if (Application.platform != RuntimePlatform.Android)
            return;

        if (isConnected)
            BluetoothConnector.CallStatic("StopConnection");
    }

    //public void ConnectionStatus(string status)
    //{
    //    Toast("Connection Status: " + status);
    //    isConnected = status == "connected";
    //}
    private void SavePairedDevice(string mac)
    {
        if (DataManager.Instance.deviceData.ContainsKey(mac))
        {
            Debug.Log($"🔁 이미 저장된 기기: {mac}");
            return;
        }

        // 이름 가져오기: UI에 있는 이름 사용하거나 fallback
        string bleName = "UNKNOWN_DEVICE";

        // UI에서 이름 찾기
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


    public void ConnectionStatus(string status)
    {
        Toast("Connection Status: " + status);
        isConnected = status == "connected";

        if (isConnected && currentConnectingMac != null)
        {
            SavePairedDevice(currentConnectingMac);
            currentConnectingMac = null; // ✅ 저장 후 초기화
        }
    }
    public void GetPairedDevices()
    {
        if (Application.platform != RuntimePlatform.Android)
            return;

        Toast("📡 페어링된 기기 스캔 중...");
        string[] data = BluetoothConnector.CallStatic<string[]>("GetPairedDevices");

        // UI 정리
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

            // UI 생성
            GameObject newDevice = Instantiate(pairedDevice, pairedListContainer.transform);
            newDevice.name = d;
            newDevice.GetComponentInChildren<TextMeshProUGUI>().text = BLEName;

            // 🔐 저장 처리: 페어링된 기기만 저장
            if (!DataManager.Instance.deviceData.ContainsKey(mac))
            {
                var newDeviceData = new DeviceData
                {
                    Mac = mac,
                    index = DataManager.Instance.deviceData.Count + 1,
                    name = BLEName,
                    description = " ",
                    autoPaired = false
                };

                DataManager.Instance.deviceData[mac] = newDeviceData;
                Debug.Log($"✅ 페어링 기기 저장됨: {mac} / {BLEName}");
            }
        }

        // 마지막에 JSON으로 저장
        DataManager.Instance.SaveData();

        addDevice.transform.SetAsLastSibling();
    }
    //public void GetPairedDevices()
    //{
    //    if (Application.platform != RuntimePlatform.Android)
    //        return;

    //    Toast("pairedDevice스캔중");
    //    string[] data = BluetoothConnector.CallStatic<string[]>("GetPairedDevices");

    //    for (int i = pairedListContainer.transform.childCount - 1; i >= 0; i--)
    //    {
    //        Transform child = pairedListContainer.transform.GetChild(i);
    //        if (!child.name.Contains("AddDevice"))
    //            Destroy(child.gameObject);
    //    }

    //    foreach (var d in data)
    //    {
    //        Toast($"{d}");
    //        int macLength = 18;
    //        GameObject newDevice = Instantiate(pairedDevice, pairedListContainer.transform);

    //        string BLEName = d.Substring(macLength).Trim().ToUpper();

    //        if (BLEName == "null" || string.IsNullOrEmpty(BLEName))
    //        {
    //            BLEName = "알 수 없는 기기";
    //        }

    //        newDevice.name = d;
    //        newDevice.GetComponentInChildren<TextMeshProUGUI>().text = BLEName;
    //    }
    //    addDevice.transform.SetAsLastSibling();
    //}

    public void ReadData(string data)
    {
        Debug.Log("BT Stream: " + data);
        receivedData.text = data;
    }

    public void WriteData(string text)
    {
        if (Application.platform != RuntimePlatform.Android)
            return;

        if (isConnected)
            BluetoothConnector.CallStatic("WriteData", text);
    }

    public void Toast(string data)
    {
        if (Application.platform != RuntimePlatform.Android)
            return;

        BluetoothConnector.CallStatic("Toast", data);
    }

    void Update()
    {

    }

    void OnApplicationQuit()
    {
        if (isConnected)
        {
            BluetoothConnector.CallStatic("StopConnection");
        }
    }
}