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
	public GameObject Table;
	public GameObject Background;
	public List<GameObjectMap> Materials;
	public Material transparent;
	public Material dotMaterial;
	public TextMeshPro infoText;
	public StretchingBehaviour HeightStretching;
	public StretchingBehaviour WidthStretching;
	public float Scale = 1.0f;
	public GameObject Queue;
	public GameObject dotsParentGameObject;

	private RootObject root = new RootObject();
	private RootState state = new RootState();
	private Animator animator;
	private StatePresenter statePresenter;
	private BallDottedPaths dottedPaths;
	private Dictionary<string, Material> mappedMaterials = new Dictionary<string, Material>();
	private bool isPlaying = true;
	private Configuration config;
    private int animationIndex = 0;
	private List<GameObject> railSegments;
	private List<GameObject> railSegmentNormals;
	private List<GameObject> targets;
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
		
		config = ConfigurationLoader.load();
		Utility.applyConfig(ref config, ref HeightStretching, ref WidthStretching, ref Table);
		Scale = config.scale;
		
		Table.GetComponent<Renderer>().enabled = false;
		infoText.gameObject.SetActive(false);

		railSegments = drawRailSegments(ref config, ref HeightStretching, ref WidthStretching);
		railSegmentNormals = drawRailSegmentNormals(ref config, ref HeightStretching, ref WidthStretching);
		targets = drawTargets(ref config, ref HeightStretching, ref WidthStretching);

		dottedPaths = new BallDottedPaths(HeightStretching, WidthStretching, Scale, dotMaterial, dotsParentGameObject);
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
			if (this.animator != null) {
				this.animator.delete();
				this.animator = null;
			}
			
		} else if (Input.GetKeyDown(KeyCode.DownArrow)) {
			animationIndex = (animationIndex - 1) % animations.Length;
			animationIndex = animationIndex > 0 ? animationIndex : -animationIndex;
			if (this.animator != null) {
				this.animator.delete();
				this.animator = null;
			}
		}
		
		if (animator == null && animations.Length > 0) {
			animator = new Animator(root, animations[animationIndex].keyFrames, mappedMaterials, transparent, config,
			    HeightStretching, WidthStretching, Scale, Queue);
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
			toggleConfigProperties();
		} else if (Input.GetKeyDown(KeyCode.D)) {
			this.isDebug = !isDebug;
		} else if (Input.GetKeyDown(KeyCode.L)) {
			isLive = !isLive;
			AnimationService.captureState(isLive);
		} else if (Input.GetKeyDown(KeyCode.H)) {
			showBallHalos = !showBallHalos;
		} else if (Input.GetKeyDown(KeyCode.Return)) {
			AnimationService.captureState();
		} else if (Input.GetKeyDown(KeyCode.C)) {
		    Search search = new Search();
		    // TODO: maybe move to config?
            search.types = new string[] { "YELLOW", "GREEN", "BROWN", "BLUE", "PINK", "BLACK" };

            infoText.SetText("Searching");
            infoText.gameObject.SetActive(true);
            AnimationService.searchSolution(search);
		}
		
		if (statePresenter != null) {
			statePresenter.enableDebug(isDebug);
			statePresenter.showHalo(showBallHalos);
		}
		
		if (isPlaying) {
			animate(Time.deltaTime);
		}
    }
	
	private void toggleConfigProperties() {
		bool enabled = !Table.GetComponent<Renderer>().enabled;
		Table.GetComponent<Renderer>().enabled = enabled;
		foreach(var segment in railSegments) {
			segment.GetComponent<LineRenderer>().enabled = enabled;
		}
// 		foreach(var segmentNormal in railSegmentNormals) {
//             segmentNormal.GetComponent<LineRenderer>().enabled = enabled;
//         }
		foreach(var target in targets) {
			target.GetComponent<LineRenderer>().enabled = enabled;
		}
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
		
		if (this.animator != null) {
			this.animator.delete();
			this.animator = null;
		}
	}
	
	private void stateChanged(RootState state) {
		if (this.statePresenter == null) {
			statePresenter = new StatePresenter(transparent, Scale, config, HeightStretching, WidthStretching);
		}
		
		statePresenter.update(state);
		dottedPaths.stateChanged(state);
	}
	
	private List<GameObject> drawRailSegments(ref Configuration config, ref StretchingBehaviour heightStretching, ref StretchingBehaviour widthStretching) {
		List<GameObject> railSegments = new List<GameObject>();
		int index = 0;
		foreach(var segment in config.segments) {
			GameObject lineObject = new GameObject(string.Format("RailSegment{0}", index++));
			LineRenderer lRend = lineObject.AddComponent<LineRenderer>();
			lRend.enabled = false;
			lRend.material = new Material(Shader.Find("Hidden/Internal-Colored"));
			lRend.startColor = Color.white;
			lRend.endColor = Color.white;
			lRend.startWidth = 0.02f;
			lRend.endWidth = 0.02f;
			lRend.SetPosition(0, position(convert(segment.start, -0.01f), heightStretching, widthStretching));
			lRend.SetPosition(1, position(convert(segment.end, -0.01f), heightStretching, widthStretching));
			lRend.sortingOrder = 0;
			railSegments.Add(lineObject);
		}
		return railSegments;
	}

	private List<GameObject> drawRailSegmentNormals(ref Configuration config, ref StretchingBehaviour heightStretching, ref StretchingBehaviour widthStretching) {
        List<GameObject> railSegmentNormals = new List<GameObject>();
        int index = 0;
        foreach(var segment in config.segments) {
            GameObject lineObject = new GameObject(string.Format("RailSegmentNormal{0}", index++));
            LineRenderer lRend = lineObject.AddComponent<LineRenderer>();
            lRend.enabled = false;
            lRend.material = new Material(Shader.Find("Hidden/Internal-Colored"));
            lRend.startColor = Color.red;
            lRend.endColor = Color.blue;
            lRend.startWidth = 0.02f;
            lRend.endWidth = 0.02f;
            Vec2 normal = segment.normal();
            Vec2 startPoint = segment.start + (0.5f * (segment.end - segment.start));
            Vec2 endPoint = startPoint + 0.1 * normal;
            lRend.SetPosition(0, position(convert(startPoint, -0.01f), heightStretching, widthStretching));
            lRend.SetPosition(1, position(convert(endPoint, -0.01f), heightStretching, widthStretching));
            lRend.sortingOrder = 0;
            railSegmentNormals.Add(lineObject);
        }
        return railSegmentNormals;
    }
	
	private List<GameObject> drawTargets(ref Configuration config, ref StretchingBehaviour heightStretching, ref StretchingBehaviour widthStretching) {
		List<GameObject> targets = new List<GameObject>();
		int index = 0;
		foreach(var target in config.targets) {
			GameObject lineObject = new GameObject(string.Format("Target{0}", index++));
			List<GameObject> lineObjectList = new List<GameObject>();
			lineObjectList.Add(lineObject);
			LineRenderer lRend = lineObject.AddComponent<LineRenderer>();
			lRend.enabled = false;
			lRend.material = new Material(Shader.Find("Hidden/Internal-Colored"));
			lRend.startColor = Color.white;
			lRend.endColor = Color.white;
			lRend.startWidth = 0.02f;
			lRend.endWidth = 0.02f;
			lRend.sortingOrder = 0;
			CreatePoints(lRend, 50, config.scale, target, ref heightStretching, ref widthStretching);
			railSegments.Add(lineObject);
		}
		return targets;
	}
	
	private void CreatePoints (LineRenderer circleRenderer, int segments, float scale, Circle circle, ref StretchingBehaviour heightStretching, ref StretchingBehaviour widthStretching) {
        float x;
        float y;

        float angle = 0f;
		float angleStep = 360f / segments;
		circleRenderer.positionCount = segments + 2;

        for (int i = 0; i < (segments + 2); i++)
        {
			var movedPos = position(convert(circle.position, -0.01f), heightStretching, widthStretching);
            y = Mathf.Cos (Mathf.Deg2Rad * angle) * (circle.radius * scale) + movedPos.y;
            x = Mathf.Sin (Mathf.Deg2Rad * angle) * (circle.radius * scale) + movedPos.x;

            circleRenderer.SetPosition(i, new Vector3(x, y, -0.01f));

            angle += angleStep;
        }
    }
	
	private static Vector3 convert(Vec2 vector, float z) {
		return new Vector3((float)vector.x, (float)vector.y, z);
	}
	
	private static Vector3 position(Vector3 pos, StretchingBehaviour heightStretching, StretchingBehaviour widthStretching) {
		return new Vector3(widthStretching.forward(pos.x), heightStretching.forward(pos.y), pos.z);
	}
	
	private static Vec2 invPosition(Vec2 pos, StretchingBehaviour heightStretching, StretchingBehaviour widthStretching) {
		return new Vec2{x = widthStretching.inverse((float) pos.x), y = heightStretching.inverse((float) pos.y)};
	}
	
	private class StatePresenter {
		private readonly List<GameObject> ballObjects = new List<GameObject>();
		private readonly StretchingBehaviour heightStretching;
		private readonly StretchingBehaviour widthStretching;
		private readonly float scale;
		private readonly Configuration config;
		private readonly Material transparent;
		
		public StatePresenter(Material transparent,
			float scale, Configuration config, StretchingBehaviour heightStretching, StretchingBehaviour widthStretching) {
			this.heightStretching = heightStretching;
			this.widthStretching = widthStretching;
			this.scale = scale;
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
			ballObject.transform.position = position(convert(ball.position, -0.01f), heightStretching, widthStretching);
			float radius = config.radius;
			ballObject.transform.localScale = new Vector3((float) radius, (float) radius, (float) radius) * 2 * scale;
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
			var invPos = invPosition(pos, heightStretching, widthStretching);
			textObject.GetComponent<TMPro.TextMeshPro>().SetText(string.Format("[{0:F4}; {1:F4}]\n{2}", invPos.x, invPos.y, id));
		}
	}
	
	private class Animator {
		
		private static readonly float QUEUE_DIST = 0.1f; // 10 cm
		private static readonly float QUEUE_DISPLAY_TIME = 1f; // 2s
		
		private readonly KeyFrame[] frames;
		private readonly Dictionary<string, GameObject> ballObjects = new Dictionary<string, GameObject>();
		private readonly List<GameObject> lines = new List<GameObject>();
		private readonly StretchingBehaviour heightStretching;
		private readonly StretchingBehaviour widthStretching;
		private readonly GameObject queue;
		private readonly float scale;
		private readonly Dictionary<string, Material> mappedMaterials;
		
		private int startFrameIndex;
		private bool animateQueue;
		private double time;
		private double endTime;
		private bool activeByDefinition = true;
		private double windowStartTime;
		private int currentAnimationWindow;

		public Animator(RootObject root, KeyFrame[] frames, Dictionary<string, Material> mappedMaterials, Material transparent,
		    Configuration config, StretchingBehaviour heightStretching, StretchingBehaviour widthStretching,
		    float scale, GameObject queue) {
			this.frames = frames;
			this.heightStretching = heightStretching;
			this.widthStretching = widthStretching;
			this.queue = queue;
			this.scale = scale;
			this.mappedMaterials = mappedMaterials;
			foreach (var ball in frames[0].balls) {
				var ballObject = GameObject.CreatePrimitive(PrimitiveType.Sphere);
				ballObject.GetComponent<MeshRenderer>().material = transparent;
				BallObjectInformation ballInfo = ballObject.AddComponent<BallObjectInformation>();
				ballInfo.id = ball.id;
				ballInfo.type = ball.type;
				ballInfo.selectable = false;
				ballObject.transform.position = position(convert(ball.position, 0), heightStretching, widthStretching);
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
							lRend.SetPosition(0, position(convert(startBall.position, -0.01f), heightStretching, widthStretching));
							lRend.SetPosition(1, position(convert(endBall.position, -0.01f), heightStretching, widthStretching));
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
			activeByDefinition = !activeByDefinition;
			foreach (var ballObject in ballObjects.Values) {
				ballObject.SetActive(activeByDefinition);
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
			Ball whiteBall = null;
			foreach (var ball in start.balls) {
				if (ball.type == "WHITE") {
					whiteBall = ball;
					break;
				}
			}
			
			if (whiteBall == null) {
				return 0.0;
			}
			
			var endSpeed = convert(whiteBall.velocity, 0);
			var direction = Vector3.Normalize(endSpeed);
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
			Ball whiteBall = null;
			foreach (var ball in start.balls) {
				if (ball.type == "WHITE") {
					whiteBall = ball;
					break;
				}
			}
			
			if (whiteBall == null) {
				return false;
			}
			
			var endSpeed = convert(whiteBall.velocity, 0);
			var direction = Vector3.Normalize(endSpeed);
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
			queue.transform.position = position(currentPos, heightStretching, widthStretching) - toMove;
			
			bool isFinished = time == totalTime;
			
			return !isFinished;
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
			gameObject.transform.position = position(0.5f * a * (float)(timeDelta * timeDelta) + startVelocity * (float)timeDelta + convert(start.position, 0),
				heightStretching, widthStretching);			
		}
	}

	private class BallDottedPaths {

    	private float ttl = 2.0f;    // in seconds
    	private float radius = 5.0f / 1000.0f; // in meters
    	private bool showDots = false;
    	private float scale = 0.05f;
    	private Material material;
    	private GameObject parentGameObject;

    	private StretchingBehaviour heightStretching;
        private StretchingBehaviour widthStretching;

        public BallDottedPaths(StretchingBehaviour heightStretching,
                               StretchingBehaviour widthStretching,
                               float scale,
                               Material material,
                               GameObject parentGameObject) {
            this.heightStretching = heightStretching;
            this.widthStretching = widthStretching;
            this.scale = scale;
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
                sphere.transform.position   = position(convert(ball.position, -0.01f), heightStretching, widthStretching);
                sphere.transform.localScale = new Vector3((float) radius, (float) radius, (float) radius) * 2 * scale;
                Destroy(sphere, ttl);
            }
        }

        private static Vector3 convert(Vec2 vector, float z) {
            return new Vector3((float)vector.x, (float)vector.y, z);
        }

        private static Vector3 position(Vector3 pos, StretchingBehaviour heightStretching, StretchingBehaviour widthStretching) {
            return new Vector3(widthStretching.forward(pos.x), heightStretching.forward(pos.y), pos.z);
        }
    }
}
