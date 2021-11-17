using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using TMPro;
using UnityEngine.SceneManagement;

[System.Serializable]
public struct GameObjectMap
{
    public string key;
    public Material material;
}

public class TableBehaviour : MonoBehaviour
{
	public float FastFactor = 2f;
	public GameObject Background;
	public List<GameObjectMap> Materials;
	public Material transparent;
	public Material dotMaterial;
	public TextMeshPro infoText;
	public TextMeshPro stabilizationStatusInfoText;
	public TableVisuals visuals;
	public GameObject Queue;
	public GameObject dotsParentGameObject;
	public StabilizationController stabilizationController;

	private RootObject root = new RootObject();
	private RootState state = new RootState();
	private Animator animator;
	private StatePresenter statePresenter;
	private BallDottedPaths dottedPaths;
	private Dictionary<string, Material> mappedMaterials = new Dictionary<string, Material>();
	private bool isPlaying = true;
	private Configuration config;
    private int animationIndex = 0;
	private bool isLive = true;
	private bool isDebug = false;
	private bool showBallHalos = true;

	// Start is called before the first frame update
    void Start()
    {
		SceneManager.SetActiveScene(SceneManager.GetSceneByName("MainScene"));
		Queue.SetActive(false);
		
		foreach(var entry in Materials)
        {
            if (!mappedMaterials.ContainsKey(entry.key))
            {
                mappedMaterials.Add(entry.key, entry.material);
            }
        }
		
		AnimationService.OnAnimationReceived += animationChanged;
		AnimationService.OnStateReceived += stateChanged;
		AnimationService.captureState(isLive);

		stabilizationController.OnStateStabilizationChange += stabilizationChanged;

		config = ConfigurationLoader.load();

		infoText.gameObject.SetActive(false);

		dottedPaths = new BallDottedPaths(dotMaterial, dotsParentGameObject);
    }
	
	void OnDestroy() {
		AnimationService.OnAnimationReceived -= animationChanged;
		AnimationService.OnStateReceived -= stateChanged;
	}

    void Update()
    {	
		
		AnimationModel[] animations = root.animations;
		
		if (Input.GetKeyDown(KeyCode.UpArrow)) {
			animationIndex = (animationIndex + 1) % animations.Length;
			recreateAnimator();
			
		} else if (Input.GetKeyDown(KeyCode.DownArrow)) {
			animationIndex = (animationIndex - 1) % animations.Length;
			animationIndex = animationIndex > 0 ? animationIndex : -animationIndex;
			recreateAnimator();
		}
		
		if (animator == null && animations.Length > 0) {
			animator = new Animator(root, animations[animationIndex].keyFrames, mappedMaterials, transparent, config, Queue);
		}
		
		if (Input.GetKeyDown(KeyCode.Space)) {
            isPlaying = !isPlaying;
        } else if (Input.GetKeyDown(KeyCode.Backspace)) {
            if (animator != null) {
				animator.reset();
				animator.update(0);
			}
        } else if (Input.GetKey(KeyCode.RightArrow)) {
			animate(Time.deltaTime * FastFactor);
		} else if (Input.GetKey(KeyCode.LeftArrow)) {
			animate(Time.deltaTime * -FastFactor);
		} else if (Input.GetKeyDown(KeyCode.K)) {
			isPlaying = false;
			if (animator != null) {
				animator.reset();
				animator.update(0);
				animator.toggleBalls();
			}
		} else if (Input.GetKeyDown(KeyCode.T)) {
			visuals.toggleTableAndRailsAndTargets();
		} else if (Input.GetKeyDown(KeyCode.D)) {
			this.isDebug = !isDebug;
        } else if (Input.GetKeyDown(KeyCode.P)) {
			this.dottedPaths.showDots = !this.dottedPaths.showDots;
		} else if (Input.GetKeyDown(KeyCode.L)) {
			isLive = !isLive;
			AnimationService.captureState(isLive);
		} else if (Input.GetKeyDown(KeyCode.H)) {
			showBallHalos = !showBallHalos;
		} else if (Input.GetKeyDown(KeyCode.M)) {
            stabilizationController.track = !stabilizationController.track;
        } else if (Input.GetKeyDown(KeyCode.Return)) {
			AnimationService.captureState();
		} else if (Input.GetKeyDown(KeyCode.R)) {
            AnimationService.videoCapture();
        } else if (Input.GetKeyDown(KeyCode.C)) {
		    Search search = new Search();
            search.types = config.coloredSearchTypes;
            Debug.Log("[TableBehaviour] Search with colored balls: " + string.Join(", ", search.types));
            searchSolution(search);
		}
		
		if (statePresenter != null) {
			statePresenter.enableDebug(isDebug);
			statePresenter.showHalo(showBallHalos);
		}
		
		if (isPlaying) {
			animate(Time.deltaTime);
		}
    }

	private void searchSolution(Search search) {
        infoText.SetText("Searching");
        infoText.gameObject.SetActive(true);
        AnimationService.searchSolution(search);
    }

	private void animate(double deltaTime) {
		if (animator != null) {
			animator.update(deltaTime);
		}
	}
	
	private void animationChanged(RootObject root) {
		infoText.SetText("");
		infoText.gameObject.SetActive(false);

		this.animationIndex = 0;
		this.root = root;

		Debug.Log("[animationChanged] animations received: " + root.animations.Length);
		
		recreateAnimator();
	}
	
	private void stateChanged(RootState state) {
		if (this.statePresenter == null) {
			statePresenter = new StatePresenter(transparent, config);
		}
		
		statePresenter.update(state);
		dottedPaths.stateChanged(state);
	}

	private void recreateAnimator() {
        if (this.animator != null) {
            this.animator.delete();
            this.animator = null;
        }
    }

	private void stabilizationChanged(StabilizationChange change) {

        if (change.current == StabilizationStatus.STABLE) {
            // -> STABLE

             this.root = new RootObject();
             recreateAnimator();

             Search search = new Search();
             search.types = config.infinityModeSearchTypes;

             Debug.Log("[Infinity-mode] STABLE: Search for solution: " + string.Join(", ", search.types));
             searchSolution(search);

        } else {

            Debug.Log("[Infinity-mode] " + change.previous + " -> " + change.current + ": Do nothing");
        }

        stabilizationStatusInfoText.SetText("" + change.current);
    }

	private static Vector3 convert(Vec2 vector, float z) {
		return new Vector3((float)vector.x, (float)vector.y, z);
	}

	private class StatePresenter {
		private readonly List<GameObject> ballObjects = new List<GameObject>();
		private readonly Configuration config;
		private readonly Material transparent;
		
		public StatePresenter(Material transparent, Configuration config) {
			this.config = config;
			this.transparent = transparent;
		}
		
				
		public void update(RootState state) {
			for (int i = 0; i < state.balls.Length; i++) {
				if (i > (ballObjects.Count - 1)) {
					append();
				}
				
				update(state.balls[i], ballObjects[i]);
			}
			
			for (int i = ballObjects.Count - 1; i >= state.balls.Length; i--) {
				var gameObject = ballObjects[i];
				Destroy(gameObject);
				ballObjects.RemoveAt(i);
			}
		}
		
		private void update(BallState ball, GameObject ballObject) {
			BallObjectInformation ballInfo = ballObject.GetComponent<BallObjectInformation>();
			ballInfo.id = ball.id;
			ballInfo.type = ball.type;
			ballInfo.selectable = true;
			ballObject.transform.position = StretchingUtility.get().position(convert(ball.position, -0.01f));
			float radius = config.radius;
			ballObject.transform.localScale = new Vector3((float) radius, (float) radius, (float) radius) * 2 * StretchingUtility.get().scale;
			updateLocationText(ball.id, ball.type, ballObject);
		}
		
		private void append() {
			var ballObject = GameObject.CreatePrimitive(PrimitiveType.Sphere);
			ballObject.GetComponent<MeshRenderer>().material = transparent;
			ballObject.AddComponent<BallObjectInformation>();
				
			GameObject textObject = new GameObject("Text");
			TMPro.TextMeshPro textMesh = textObject.AddComponent<TMPro.TextMeshPro>();
			textMesh.fontSize = 4;
			textMesh.GetComponent<MeshRenderer >().enabled = false;
			textObject.transform.SetParent(ballObject.transform, false);
			textObject.transform.localPosition = new Vector3(0, -2, 0);
			textObject.GetComponent<RectTransform>().sizeDelta = new Vector2(5, 2);
			
			ballObjects.Add(ballObject);
		}
		
		public void enableDebug(bool debug) {
			foreach(var ball in ballObjects) {
            	var textObject = ball.transform.Find("Text").gameObject;
            	textObject.GetComponent<TMPro.TextMeshPro>().GetComponent<MeshRenderer>().enabled = debug;
            }
		}
		
		public void showHalo(bool show) {
			if (show) {
				Utility.drawCircle(ballObjects, 1, 50);
			} else {
				foreach (var ball in ballObjects) {
					Destroy(ball.GetComponent<LineRenderer>());
				}
			}
		}

		private void updateLocationText(string id, string type, GameObject ball) {
			var textObject = ball.transform.Find("Text").gameObject;
			var pos = new Vec2{x = ball.transform.position.x, y = ball.transform.position.y};
			var invPos = StretchingUtility.get().invPosition(pos);
			textObject.GetComponent<TMPro.TextMeshPro>().SetText(string.Format("[{0:F4}; {1:F4}]\n{2}", invPos.x, invPos.y, id));
		}
	}

	private class Animator {
		
		private static readonly float QUEUE_DIST = 0.1f; // 10 cm
		private static readonly float QUEUE_DISPLAY_TIME = 1f; // 2s
		
		private readonly KeyFrame[] frames;
		private readonly Dictionary<string, GameObject> ballObjects = new Dictionary<string, GameObject>();
		private readonly List<GameObject> lines = new List<GameObject>();
		private readonly GameObject queue;
		private readonly Dictionary<string, Material> mappedMaterials;
		private readonly Material transparent;

		private int startFrameIndex;
		private bool animateQueue;
		private double time;
		private double endTime;
		private double windowStartTime;
		private int currentAnimationWindow;
		private BallMaterial material = BallMaterial.CIRCLE;

		public Animator(RootObject root, KeyFrame[] frames, Dictionary<string, Material> mappedMaterials, Material transparent,
		    Configuration config, GameObject queue) {
			this.frames = frames;
			this.queue = queue;
			this.mappedMaterials = mappedMaterials;
			this.transparent = transparent;
			float scale = StretchingUtility.get().scale;
			foreach (var ball in frames[0].balls) {
				var ballObject = GameObject.CreatePrimitive(PrimitiveType.Sphere);
				ballObject.GetComponent<MeshRenderer>().material = transparent;
				BallObjectInformation ballInfo = ballObject.AddComponent<BallObjectInformation>();
				ballInfo.id = ball.id;
				ballInfo.type = ball.type;
				ballInfo.selectable = false;
				ballObject.transform.position = StretchingUtility.get().position(convert(ball.position, 0));
				float radius = config.radius;
				ballObject.transform.localScale = new Vector3((float) radius, (float) radius, (float) radius) * 2 * scale;
				
				List<GameObject> objectList = new List<GameObject>();
				objectList.Add(ballObject);
				Utility.drawCircle(objectList, (radius * scale) / (radius * scale * 2) - Utility.LINE_WIDTH * 2, 50);
				
				var circleRenderer = ballObject.GetComponent<LineRenderer>();
				circleRenderer.material = mappedMaterials[ball.type];
				circleRenderer.useWorldSpace = false;
				ballObjects.Add(ball.id, ballObject);
			}
			
			reset();
		}

		private enum BallMaterial {
            CIRCLE,
            FILLED,
            INVISIBLE
        }

		public void drawLines() {
			foreach (var line in lines) {
				line.SetActive(false);
				Destroy(line);
			}
			lines.Clear();
			
			int windowCount = -1;
			for (int i = 0; i < this.frames.Length - 1; i+=2) {
				var start = this.frames[i];
				if (start.firstFrame) {
					windowCount++;
				}
				
				if (windowCount == currentAnimationWindow) {
					var end = this.frames[i + 1];
				
					foreach (var startBall in start.balls) {
						var endBall = findBall(startBall.id, end);
						if (endBall != null && startBall.position != endBall.position) {
							GameObject lineObject = new GameObject(string.Format("Line{0}", startBall.id));
							LineRenderer lRend = lineObject.AddComponent<LineRenderer>();
							lRend.material = new Material(Shader.Find("Hidden/Internal-Colored"));
							lRend.material.color = Color.white;
							lRend.startWidth = 0.02f;
							lRend.endWidth = 0.02f;
							lRend.SetPosition(0, StretchingUtility.get().position(convert(startBall.position, -0.01f)));
							lRend.SetPosition(1, StretchingUtility.get().position(convert(endBall.position, -0.01f)));
							lines.Add(lineObject);
						}
					}
				} else if (windowCount > currentAnimationWindow) {
					break;
				}
			}
		}
		
		public void reset() {
			this.time = 0;
			this.startFrameIndex = 0;
			this.currentAnimationWindow = 0;
			this.windowStartTime = 0;
			this.animateQueue = true;
			
			KeyFrame start = this.frames[this.startFrameIndex];
			KeyFrame end = this.frames[this.startFrameIndex + 1];
			updateBalls(start, end, 0);
			
			drawLines();
		}
		
		public void delete() {
			foreach (var ballObject in ballObjects.Values) {
				ballObject.SetActive(false);
				Destroy(ballObject);
			}
			ballObjects.Clear();
			
			foreach (var line in lines) {
				line.SetActive(false);
				Destroy(line);
			}
			lines.Clear();
		}
		
		public void update(double timeDelta) {
			double newTime = this.time + timeDelta;
			this.time = Math.Min(Math.Max(0, newTime), this.endTime);
			
			KeyFrame start = this.frames[this.startFrameIndex];
			
			if (animateQueue) {
				queue.SetActive(true);
				double queueEndTime = calculateQueueEndTime(start);
				this.endTime = queueEndTime + this.windowStartTime;
				animateQueue = doAnimateQueue(this.time - this.windowStartTime, queueEndTime, start);
				if (!animateQueue) {
					this.time = this.windowStartTime;
					this.endTime = frames[frames.Length - 1].time;
				}
			} else {
				mayUpdateAnimationWindow(time, this.frames);
				if (!isFinished()) {
					start = this.frames[this.startFrameIndex];
					KeyFrame end = this.frames[this.startFrameIndex + 1];
					if (this.time >= (QUEUE_DISPLAY_TIME + this.windowStartTime)) {
						queue.SetActive(false);
					}
					updateBalls(start, end, time);
				} else {
					KeyFrame end = this.frames[this.startFrameIndex + 1];
					foreach (var ball in end.balls) {
						var ballObject = ballObjects[ball.id];
						ballObject.SetActive(ball.visible);
					}
				}
			}
		}
		
		public void toggleBalls() {
			switch(material) {
			    case BallMaterial.CIRCLE:
                    material = BallMaterial.FILLED;
                    break;
			    case BallMaterial.FILLED:
			        material = BallMaterial.INVISIBLE;
                    break;
			    case BallMaterial.INVISIBLE:
			        material = BallMaterial.CIRCLE;
                    break;
			}
			foreach (var ballObject in ballObjects.Values) {
				switch(material) {
                    case BallMaterial.CIRCLE:
                        ballObject.SetActive(true);
                        ballObject.GetComponent<MeshRenderer>().material = transparent;
                        break;
                    case BallMaterial.FILLED:
                        ballObject.SetActive(true);
                        ballObject.GetComponent<MeshRenderer>().material = mappedMaterials[ballObject.GetComponent<BallObjectInformation>().type];
                        break;
                    case BallMaterial.INVISIBLE:
                        ballObject.SetActive(false);
                        break;
                }
			}
		}
		
		public bool isFinished() {
			return this.time == this.endTime;
		}
		
		private void updateBalls(KeyFrame start, KeyFrame end, double time) {
			double duration = end.time - start.time;
			double timeDeltaInAnimationWindow = time - start.time;
			
			var unvisited = ballObjects.Keys.ToList(); 
			foreach (var startBall in start.balls) {
				var ballObject = ballObjects[startBall.id];
				if (startBall.visible) {
					var endBall = findBall(startBall.id, end);
							
					ballObject.SetActive(true);
					updatePosition(startBall, endBall, ballObject, duration, timeDeltaInAnimationWindow);
				} else {
					ballObject.SetActive(false);
				}
				unvisited.Remove(startBall.id);
			}
			
			foreach (var id in unvisited) {
				var ballObject = ballObjects[id];
				ballObject.SetActive(false);
			}
		}
		
		private double calculateQueueEndTime(KeyFrame start) {
			Ball whiteBall = findCueBall(start.balls);
			if (whiteBall == null) {
				return 0.0;
			}
			
			var endSpeed = convert(whiteBall.velocity, 0);
			var direction = Vector3.Normalize(endSpeed);
			float scale = StretchingUtility.get().scale;
			float radius = (ballObjects[whiteBall.id].transform.localScale.x / scale) / 2;
			
			var startDirection = direction * (QUEUE_DIST + radius);
			direction = direction * (QUEUE_DIST);
			var startPos = convert(whiteBall.position, 0) - startDirection;
			var endPos = startPos + direction;
			
			// endPos = a/2 * t^2 + startPos
			// endPos = (endSpeed / t)/2 * t^2 + startPos
			// endPos = (endSpeed / 2*t) * t^2 + startPos
			// endPos = (endSpeed / 2) * t + startPos
			// endPos - startPos = (endSpeed / 2) * t
			// (endPos - startPos)/(endSpeed / 2) = t
			return Math.Abs((endPos - startPos).magnitude / (endSpeed / 2.0f).magnitude);
		}
		
		private bool doAnimateQueue(double time, double totalTime, KeyFrame start) {
			Ball whiteBall = findCueBall(start.balls);
			if (whiteBall == null) {
				return false;
			}

			var endSpeed = convert(whiteBall.velocity, 0);
			var direction = Vector3.Normalize(endSpeed);
			float scale = StretchingUtility.get().scale;
			float radius = (ballObjects[whiteBall.id].transform.localScale.x / scale) / 2;
			
			var startDirection = direction * (QUEUE_DIST + radius);
			var startPos = convert(whiteBall.position, 0) - startDirection;

			var a = endSpeed / (float)totalTime;
			time = Math.Min(time, totalTime);
			var currentPos = a / 2 * (float)(time * time) + startPos;
			
			float degrees = (float)(180 / Math.PI) * (float) Math.Acos(Vector3.Dot(new Vector3(0, -1, 0), endSpeed) / (endSpeed.magnitude));
			queue.transform.eulerAngles = new Vector3(0, 0, endSpeed.x > 0 ? degrees : -degrees);
			
			float lengthToMove = queue.transform.localScale.y;
			var toMove = Vector3.Normalize(endSpeed) * lengthToMove;
			queue.transform.position = StretchingUtility.get().position(currentPos) - toMove;
			
			bool isFinished = time == totalTime;
			
			return !isFinished;
		}

		private Ball findCueBall(Ball[] balls) {
		    foreach (var ball in balls) {
                if (ball.type == "WHITE" && (ball.velocity.x != 0.0 || ball.velocity.y != 0.0)) {
                    return ball;
                }
            }
            return null;
		}

		private void mayUpdateAnimationWindow(double time, KeyFrame[] frames) {
			KeyFrame currentEndFrame = frames[this.startFrameIndex + 1];
			KeyFrame currentStartFrame = frames[this.startFrameIndex];
			if (currentEndFrame.time < time) {
				this.startFrameIndex += 2;
				if (frames[this.startFrameIndex].firstFrame) {
					this.currentAnimationWindow++;
					this.animateQueue = true;
					this.windowStartTime = frames[this.startFrameIndex].time;
					drawLines();
				}
			} else if (currentStartFrame.time > time && time >= 0) {
				this.startFrameIndex -= 2;
				if (this.startFrameIndex < frames.Length - 1) {
					if (frames[this.startFrameIndex + 2].firstFrame) {
						this.currentAnimationWindow--;
						for (int i = startFrameIndex; i >= 0; i--) {
							if (frames[i].firstFrame) {
								this.windowStartTime = frames[i].time;
								break;
							}
						}
						drawLines();
					}				
				}
			}
		}
		
		private Ball findBall(string id, KeyFrame frame) {
			foreach (var ball in frame.balls) {
				if (ball.id == id) {
					return ball;
				}
			}
			return null;
		}
		
		private void updatePosition(Ball start, Ball end, GameObject gameObject, double duration, double timeDelta) {
			Vector3 startVelocity = convert(start.velocity,0.0f);
			Vector3 endVelocity = convert(end.velocity,0.0f);
			Vector3 a = duration == 0.0f ? new Vector3(0.0f, 0.0f, 0.0f) : (endVelocity - startVelocity) * (float)(1/duration);
			gameObject.transform.position = StretchingUtility.get().position(0.5f * a * (float)(timeDelta * timeDelta) + startVelocity * (float)timeDelta + convert(start.position, 0));
		}
	}

	private class BallDottedPaths {

    	private float ttl = 2.0f;    // in seconds
    	private float radius = 5.0f / 1000.0f; // in meters
    	public bool showDots = true;
    	private Material material;
    	private GameObject parentGameObject;

        public BallDottedPaths(Material material,
                               GameObject parentGameObject) {
            this.material = material;
            this.parentGameObject = parentGameObject;
        }

        public void stateChanged(RootState state) {

            if (!showDots) {
                return;
            }

            foreach (BallState ball in state.balls) {
                GameObject sphere = GameObject.CreatePrimitive(PrimitiveType.Sphere);
                sphere.GetComponent<MeshRenderer>().material = material;
                sphere.transform.parent     = parentGameObject.transform;
                sphere.transform.position   = StretchingUtility.get().position(convert(ball.position, -0.01f));
                sphere.transform.localScale = new Vector3(radius, radius, radius) * 2 * StretchingUtility.get().scale;
                Destroy(sphere, ttl);
            }
        }

        private static Vector3 convert(Vec2 vector, float z) {
            return new Vector3((float)vector.x, (float)vector.y, z);
        }
    }
}
