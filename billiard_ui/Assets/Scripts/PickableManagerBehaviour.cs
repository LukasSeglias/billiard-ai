using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using TMPro;

public class PickableManagerBehaviour : MonoBehaviour
{
	public TextMeshPro infoText;

    void Update()
    {
        if (Input.GetMouseButtonDown(0)){ // Left click
			maySearch(SearchBy.ID);
		} else if (Input.GetMouseButtonDown(1)){ // Right click
			maySearch(SearchBy.TYPE);
		}
    }
	
	private void maySearch(SearchBy searchBy) {
		RaycastHit hitInfo = new RaycastHit();
		bool hit = Physics.Raycast(Camera.main.ScreenPointToRay(Input.mousePosition), out hitInfo);

		if (hit) {
			var gameObject = hitInfo.transform.gameObject;
			var ballInfo = gameObject.GetComponent<BallObjectInformation>();

			if (ballInfo != null && ballInfo.selectable) {
				Search search = new Search();
				switch(searchBy) {
					case SearchBy.ID:
						search.id = ballInfo.id;
						break;
					case SearchBy.TYPE:
					default:
						search.types = new string[] { ballInfo.type };
						break;
					}

					infoText.SetText("Searching");
					infoText.gameObject.SetActive(true);
					AnimationService.searchSolution(search);	
			}
		}
	}
	
	private enum SearchBy {
		ID,
		TYPE
	}
}
