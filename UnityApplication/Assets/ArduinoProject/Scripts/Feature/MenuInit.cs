using UnityEngine;

public class MenuInit : MonoBehaviour
{
    public ScrollSnap scrollSnap;

    void Start()
    {
        if (scrollSnap != null)
        {
            scrollSnap.OnIndexChanged += OnScrollIndexChanged;
        }
    }

    void OnScrollIndexChanged(int newIndex)
    {
        
    }

    void OnDestroy()
    {
        if (scrollSnap != null)
        {
            scrollSnap.OnIndexChanged -= OnScrollIndexChanged;
        }
    }
}
