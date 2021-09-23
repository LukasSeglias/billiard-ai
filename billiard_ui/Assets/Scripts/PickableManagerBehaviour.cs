using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using TMPro;

public class PickableManagerBehaviour : MonoBehaviour
{
	public TextMeshPro infoText;
	// https://gamedev.stackexchange.com/questions/126427/draw-circle-around-gameobject-to-indicate-radius
	[Range(0,50)]
    public int segments = 50;
    [Range(0,5)]
    public float radius = 1;
	
	private LineRenderer circleRenderer;
	private GameObject currentSelected;
	
    void Update(){
		clear();
		
        if (Input.GetMouseButtonDown(0)){ 
			RaycastHit hitInfo = new RaycastHit();
			bool hit = Physics.Raycast(Camera.main.ScreenPointToRay(Input.mousePosition), out hitInfo);
			if (hit) {
				var gameObject = hitInfo.transform.gameObject;
				if (gameObject.GetComponent<PickableInformation>() != null) {
					var pickableInfo = gameObject.GetComponent<PickableInformation>();
					if (currentSelected != gameObject) {
						pickableInfo.picked = 0;
					}
					pickableInfo.picked = (pickableInfo.picked + 1) % 3;
					
					switch (pickableInfo.picked) {
						case 1:
							currentSelected = gameObject;
							break;
						case 2:
							currentSelected = gameObject;
							break;
						default:
							currentSelected = null;
							break;
					}
				} else {
					currentSelected = null;
				}
			} else {
				currentSelected = null;
			}
		} else if (Input.GetKeyDown(KeyCode.Return)) {
			if (currentSelected != null) {
				var selection = currentSelected.GetComponent<PickableInformation>();
				Search search = new Search();
				switch (selection.picked) {
					case 1:
						search.id = selection.id;
						break;
					case 2:
						search.type = selection.type;
						break;
					default:
						break;
				}
				infoText.SetText("Searching");
				infoText.gameObject.SetActive(true);
				AnimationService.searchSolution(search);
			} else {
				infoText.SetText("Capturing");
				infoText.gameObject.SetActive(true);
				AnimationService.captureState();
			}
		}
		
		if (currentSelected != null) {
			Utility.drawCircle(new List<GameObject> {currentSelected}, radius, segments);
			circleRenderer = currentSelected.GetComponent<LineRenderer>();
			
			var pickableInfo = currentSelected.GetComponent<PickableInformation>();
			if (pickableInfo.picked == 2) {
				Color matColor = circleRenderer.material.color;
				circleRenderer.material.color = new Color(matColor.r, matColor.g, matColor.b, 0.5f);
			}
		}
    }
	
	private void clear() {
		if (circleRenderer != null) {
			circleRenderer.positionCount = 0;			
		}
	}
}
