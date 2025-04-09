using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;
using System;

public class ScrollSnap : MonoBehaviour, IBeginDragHandler, IEndDragHandler
{
    #region Event
    public Action<int> OnIndexChanged;
    #endregion

    public ScrollRect scrollRect;
    public RectTransform content;
    public GameObject addDevice;
    public float snapSpeed = 10f;

    private int itemCount;
    private float itemWidth;

    private int _currentIndex = 0;

    public int currentIndex
    {
        get => _currentIndex;
        set
        {
            if (_currentIndex != value)
            {
                _currentIndex = value;
                OnIndexChanged?.Invoke(_currentIndex);
            }
        }
    }


    private Vector2 dragStartPos;
    private bool isDragging = false;

    void Start()
    {
        StartCoroutine(InitAfterLayout());
    }

    void Update()
    {
        if (!isDragging)
        {
            Vector2 targetPos = new Vector2(-currentIndex * itemWidth, content.anchoredPosition.y);
            content.anchoredPosition = Vector2.Lerp(content.anchoredPosition, targetPos, Time.deltaTime * snapSpeed);
        }
    }

    public void OnBeginDrag(PointerEventData eventData)
    {
        isDragging = true;
        dragStartPos = eventData.position;
    }

    public void OnEndDrag(PointerEventData eventData)
    {
        isDragging = false;
        float dragDelta = eventData.position.x - dragStartPos.x;

        if (Mathf.Abs(dragDelta) > itemWidth * 0.3f)
        {
            int step = Mathf.RoundToInt(dragDelta / -itemWidth);
            currentIndex += step;
        }
        currentIndex = Mathf.Clamp(currentIndex, 0, itemCount - 1);
    }

    IEnumerator InitAfterLayout()
    {
        yield return null;

        if (addDevice != null)
        {
            addDevice.transform.SetAsLastSibling();
        }

        itemCount = content.childCount;

        if (itemCount >= 2)
        {
            itemWidth = Mathf.Abs(content.GetChild(1).GetComponent<RectTransform>().anchoredPosition.x -
                                  content.GetChild(0).GetComponent<RectTransform>().anchoredPosition.x);
        }

        currentIndex = 0;
        content.anchoredPosition = new Vector2(0, content.anchoredPosition.y);
    }

}