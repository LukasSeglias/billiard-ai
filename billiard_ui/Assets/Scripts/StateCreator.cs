using System;
using System.IO;
using System.Globalization;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using TMPro;
using UnityEngine.SceneManagement;

public class StateCreator : MonoBehaviour
{
	
	public TableVisuals visuals;
	public List<GameObjectMap> Materials;
	public TextMeshPro typeText;
	public float TextVisibilityTime = 3f;
	public InputField stateInputField;

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
		stateInputField.gameObject.SetActive(false);
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
		} else if (Input.GetKeyDown(KeyCode.T)) {

		    stateInputField.gameObject.SetActive(true);

		} else if (Input.GetKeyDown(KeyCode.Return)) {

		    List<BallState> balls = new List<BallState>();
		    Vec2 velocity = new Vec2{x = 0, y = 0};

		    if (stateInputField.text.Length > 0) {
		        var parsedState = parseState(stateInputField.text);
		        balls = parsedState.Item1;
		        velocity = parsedState.Item2;
		        stateInputField.text = "";
		        stateInputField.gameObject.SetActive(false);
		    } else {
		        balls = mapToBallStates(ballObjects);
		    }

			AnimationService.setState(balls, velocity);
			foreach (var ball in ballObjects) {
				Destroy(ball);
			}
			ballObjects.Clear();
			SceneManager.UnloadSceneAsync("StateCreationScene");
			SceneManager.LoadScene("MainScene", LoadSceneMode.Additive);
		}
    }

    /**
     * Example:
     * WHITE1, WHITE, -564.147, 9.06096
     * RED2, RED, -47.218, -100.06
     * RED4, RED, 2.93182, 398.78
     * RED5, RED, -865.047, -388.453
     * RED6, RED, -629.728, 87.0048
     * RED7, RED, 851.622, 398.78
     * V, 130.33, 234.33
     */
    private static (List<BallState>, Vec2) parseState(string text) {

        List<BallState> balls = new List<BallState>();
        Vec2 velocity = null;

        using (StringReader sr = new StringReader(text)) {
            string line;
            while ((line = sr.ReadLine()) != null) {

                string[] parts = line.Split(',');
                if (parts.Length == 3 && parts[0].Trim().Equals("V", StringComparison.OrdinalIgnoreCase)) {
                    double x = double.Parse(parts[1].Trim(), CultureInfo.InvariantCulture);
                    double y = double.Parse(parts[2].Trim(), CultureInfo.InvariantCulture);
                    velocity = new Vec2{x = x, y = y};
                } else {
                    if (parts.Length != 4) {
                        return (new List<BallState>(), null);
                    }

                    string id = parts[0].Trim();
                    string type = parts[1].Trim();
                    double x = double.Parse(parts[2].Trim(), CultureInfo.InvariantCulture);
                    double y = double.Parse(parts[3].Trim(), CultureInfo.InvariantCulture);
                    balls.Add(new BallState {
                        id = id,
                        type = type,
                        position = new Vec2 { x = x, y = y }
                    });
                }
            }
        }

        return (balls, velocity);
    }

    private static List<BallState> mapToBallStates(List<GameObject> balls) {
        List<BallState> ballStates = new List<BallState>();

        foreach (GameObject ball in balls) {
            var info = ball.GetComponent<BallObjectInformation>();

            Vec2 position = 1000.0 * StretchingUtility.get().invPosition(new Vec2 {
                x = ball.transform.position.x,
                y = ball.transform.position.y
            });
            BallState ballState = new BallState {
                type = info.type,
                id = info.id,
                position = new Vec2 { x = position.x, y = position.y }
            };
            ballStates.Add(ballState);
        }
        return ballStates;
    }
}
