using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using TMPro;
using UnityEngine.SceneManagement;

public class StateCreator : MonoBehaviour
{
	
	public TableVisuals visuals;
	public List<GameObjectMap> Materials;
	public TextMeshPro typeText;
	public float TextVisibilityTime = 3f;

	private int currentCreationIndex = 0;
	private float visibleTime = 0;
	private List<GameObject> ballObjects = new List<GameObject>();
	private GameObject currentSelected;
	private float radius;
	private int id = 0;
	
    // Start is called before the first frame update
    void Start()
    {
		SceneManager.SetActiveScene(SceneManager.GetSceneByName("StateCreationScene"));
		
		Configuration config = ConfigurationLoader.load();
		radius = config.radius;

		typeText.SetText(Materials[currentCreationIndex].key);
    }

    // Update is called once per frame
    void Update() {

		if (Input.GetAxis("Mouse ScrollWheel") > 0f) {
			currentCreationIndex = (++currentCreationIndex) % Materials.Count;
		} else if (Input.GetAxis("Mouse ScrollWheel") < 0f) {
			currentCreationIndex = Math.Abs((--currentCreationIndex) % Materials.Count);
		}
		if (Input.GetAxis("Mouse ScrollWheel") != 0f ) {
			typeText.SetText(Materials[currentCreationIndex].key);
			visibleTime = 0;
			typeText.gameObject.SetActive(true);
		} else {
			visibleTime += Time.deltaTime;
			if (visibleTime >= TextVisibilityTime) {
				typeText.gameObject.SetActive(false);
			}
		}
		
		if (Input.GetButtonDown("Fire1")) {
			Vector3 mousePos = Input.mousePosition;
			mousePos.z = 1f;
			Vector3 objectPos = Camera.main.ScreenToWorldPoint(mousePos);
			
			var ballObject = GameObject.CreatePrimitive(PrimitiveType.Sphere);
			ballObject.GetComponent<MeshRenderer>().material = Materials[currentCreationIndex].material;
			BallObjectInformation ballInfo = ballObject.AddComponent<BallObjectInformation>();
			ballInfo.id = Materials[currentCreationIndex].key + id++;
			ballInfo.type = Materials[currentCreationIndex].key;
			ballObject.transform.position = objectPos;
			ballObject.transform.localScale = new Vector3(
                (float) radius/0.5f * StretchingUtility.get().scale,
                (float) radius/0.5f * StretchingUtility.get().scale,
                (float) radius/0.5f * StretchingUtility.get().scale
			);
			ballObjects.Add(ballObject);
        } else if (Input.GetMouseButtonDown(1)){ 
			RaycastHit hitInfo = new RaycastHit();
			bool hit = Physics.Raycast(Camera.main.ScreenPointToRay(Input.mousePosition), out hitInfo);
			if (hit) {
				var gameObject = hitInfo.transform.gameObject;
				if (gameObject.GetComponent<BallObjectInformation>() != null) {
					ballObjects.Remove(gameObject);
					Destroy(gameObject);
				}
			}
		} else if (Input.GetKeyDown(KeyCode.Return)) {
			AnimationService.setState(ballObjects);
			foreach (var ball in ballObjects) {
				Destroy(ball);
			}
			ballObjects.Clear();
			SceneManager.UnloadSceneAsync("StateCreationScene");
			SceneManager.LoadScene("MainScene", LoadSceneMode.Additive);
		}
    }
}
