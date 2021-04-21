using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CursorBehaviour : MonoBehaviour
{
	public int HideAfter = 3;
	public bool Hide = true;
	public GameObject Observed;
	public Texture2D cursorTexture;
	public Vector2 Center;
	
	private float visibleTime = 0;

    // Update is called once per frame
    void Update()
    {
        RaycastHit hitInfo = new RaycastHit();
		bool hit = Physics.Raycast(Camera.main.ScreenPointToRay(Input.mousePosition), out hitInfo);
		bool mouseOverObject = false;
		if (hit) {
			var gameObject = hitInfo.transform.gameObject;
			if (gameObject == Observed || gameObject.GetComponent<PickableInformation>() != null) {
				mouseOverObject = true;
				Cursor.SetCursor(cursorTexture, Center, CursorMode.Auto);
			} else {
				Cursor.SetCursor(null, Vector2.zero, CursorMode.Auto);
			}
		} else {
			Cursor.SetCursor(null, Vector2.zero, CursorMode.Auto);
		}
		
		bool showMouseCursor = false;
		if (mouseOverObject) {
			if(Input.GetAxis("Mouse X") != 0 || Input.GetAxis("Mouse Y") != 0){
				visibleTime = 0;
			}
			visibleTime = Math.Min(HideAfter, visibleTime + Time.deltaTime);
			showMouseCursor = visibleTime < HideAfter;
		}
		
		 Cursor.visible = showMouseCursor;
    }
}
