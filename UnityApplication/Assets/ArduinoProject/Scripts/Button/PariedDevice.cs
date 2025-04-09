using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class PariedConntect : MonoBehaviour
{
    public void ConnectionButton()
    {
        BluetoothManager.Instance.StartConnection(this.name.Substring(0, 17).ToUpper());
        BluetoothManager.Instance.Toast($"{this.name} ø¨∞·¡ﬂ");
    }
}
