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
	public Material dottedLineMaterial;
	public TextMeshPro infoText;
	public TextMeshPro stabilizationStatusInfoText;
	public TableVisuals visuals;
	public GameObject Queue;
	public GameObject dotsParentGameObject;
	public StabilizationController stabilizationController;

	private const float FAST_RESULT_DISPLAY_TIME = 2.0f;
	private RootState state = new RootState();
	private AnimatorState animator;
	private AnimationPlayer animationPlayer;
	private StatePresenter statePresenter;
	private BallDottedPaths dottedPaths;
	private Dictionary<string, Material> mappedMaterials = new Dictionary<string, Material>();
	private bool isPlaying = true;
	private Configuration config;
	private bool isLive = true;
	private bool isDebug = false;
	private bool showBallHalos = true;
	private bool displayResultsFast = false;
	private float timeForOneResult = 0.0f;
	private float timeSinceLastResult = 0.0f;
	private bool infinityModeActive = false;
	private TableStatus previousTableStatus = TableStatus.UNKNOWN;

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

		animator = new AnimatorState(mappedMaterials, transparent, config, Queue, dottedLineMaterial);
		animationPlayer = new AnimationPlayer(animator);
    }

	void OnDestroy() {
		AnimationService.OnAnimationReceived -= animationChanged;
		AnimationService.OnStateReceived -= stateChanged;
	}

    void Update()
    {
		if (Input.GetKeyDown(KeyCode.UpArrow)) {
		    animationPlayer.next();
		} else if (Input.GetKeyDown(KeyCode.DownArrow)) {
		    animationPlayer.previous();
		}

		if (Input.GetKeyDown(KeyCode.Space)) {
            isPlaying = !isPlaying;
        } else if (Input.GetKeyDown(KeyCode.Backspace)) {
            animator.reset();
            animator.update(0);
        } else if (Input.GetKey(KeyCode.RightArrow)) {
			animate(Time.deltaTime * FastFactor);
		} else if (Input.GetKey(KeyCode.LeftArrow)) {
			animate(Time.deltaTime * -FastFactor);
		} else if (Input.GetKeyDown(KeyCode.K)) {
			isPlaying = false;
            animator.toggleBalls();
		} else if (Input.GetKeyDown(KeyCode.O)) {
            statePresenter.toggleMaterial();
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
		    // TODO: Remove once StabilizationController is obsolete
//             stabilizationController.track = !stabilizationController.track;
//             stabilizationStatusInfoText.SetText("");
            infinityModeActive = !infinityModeActive;
        } else if (Input.GetKeyDown(KeyCode.Return)) {
			AnimationService.captureState();
		} else if (Input.GetKeyDown(KeyCode.R)) {
            AnimationService.videoCapture();
        } else if (Input.GetKeyDown(KeyCode.C)) {
		    Search search = new Search();
            search.types = config.coloredSearchTypes;
            Debug.Log("[TableBehaviour] Search with colored balls: " + string.Join(", ", search.types));
            searchSolution(search);
		} else if (Input.GetKeyDown(KeyCode.Y)) {
			AnimationService.toggleSearch();
		} else if (Input.GetKeyDown(KeyCode.X)) {
			showBallHalos = false;
			isPlaying = false;
			displayResultsFast = true;
			timeForOneResult = FAST_RESULT_DISPLAY_TIME / animationPlayer.animations();
			timeSinceLastResult = 0.0f;
			animationPlayer.showFirst();
		}

		if (statePresenter != null) {
			statePresenter.enableDebug(isDebug);
			statePresenter.showHalo(showBallHalos);
		}

		if (isPlaying) {
			animate(Time.deltaTime);
		}
		
		if (displayResultsFast) {
			if (animationPlayer.hasNext()) {
				if (timeSinceLastResult >= timeForOneResult) {
					timeSinceLastResult = 0;
					animationPlayer.next();
				} else {
					timeSinceLastResult += Time.deltaTime;
				}
			} else {
				displayResultsFast = false;
			}
		}
    }

	private void searchSolution(Search search) {
        infoText.SetText("Searching");
        infoText.gameObject.SetActive(true);
        AnimationService.searchSolution(search);
    }

	private void animate(double deltaTime) {
        animator.update(deltaTime);
	}

	private void animationChanged(RootObject root) {
		infoText.SetText("");
		infoText.gameObject.SetActive(false);

		Debug.Log("[animationChanged] animations received: " + root.animations.Length);

        animationPlayer.setAnimation(root);
	}

	private void stateChanged(RootState state) {
		if (this.statePresenter == null) {
		    Material background = Background.GetComponent<MeshRenderer>().material;
			statePresenter = new StatePresenter(transparent, background, mappedMaterials, config);
		}

		statePresenter.update(state);
		dottedPaths.stateChanged(state);

		if (infinityModeActive) {
		    updateInfinityMode(previousTableStatus, state.status);
		    previousTableStatus = state.status;
		}
	}

	private void updateInfinityMode(TableStatus previousState, TableStatus currentState) {

	    if (previousState != currentState) {
            // Status changed

            if (currentState == TableStatus.STABLE) {
                // -> STABLE

                this.animationPlayer.setAnimation(new RootObject());

                Search search = new Search();
                search.types = config.infinityModeSearchTypes;

                Debug.Log("[Infinity-mode] STABLE: Search for solution: " + string.Join(", ", search.types));
                searchSolution(search);
            }

            stabilizationStatusInfoText.SetText("" + currentState);
        }
	}

	private void stabilizationChanged(StabilizationChange change) {
        // TODO: Remove once StabilizationController is obsolete
        if (change.current == StabilizationStatus.STABLE) {
            // -> STABLE

             this.animationPlayer.setAnimation(new RootObject());

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

	private class AnimationPlayer {

	    private int animationIndex = 0;
	    private RootObject root;
	    private AnimatorState animator;

	    public AnimationPlayer(AnimatorState animator) {
	        this.animator = animator;
	    }

	    public void setAnimation(RootObject root) {
	        this.root = root;
	        this.animationIndex = 0;
	        switchToAnimation(animationIndex);
	    }

	    public void next() {
	        AnimationModel[] animations = root.animations;
            animationIndex = (animationIndex + 1) % animations.Length;
            switchToAnimation(animationIndex);
	    }
		
		public bool hasNext() {
			AnimationModel[] animations = root.animations;
			return animationIndex < animations.Length - 1;
		}
		
		public void showFirst() {
			this.animationIndex = 0;
	        switchToAnimation(animationIndex);
		}
		
		public int animations() {
			return root.animations.Length;
		}

        public void previous() {
            AnimationModel[] animations = root.animations;
            animationIndex = (animationIndex - 1) % animations.Length;
            animationIndex = animationIndex > 0 ? animationIndex : -animationIndex;
            switchToAnimation(animationIndex);
	    }

	    private void switchToAnimation(int animationIndex) {
	        if (root.animations.Length > 0) {
                setFrames(root.animations[animationIndex].keyFrames);
            } else {
                setFrames(new KeyFrame[]{});
            }
        }

	    private void setFrames(KeyFrame[] frames) {
	        animator.setAnimation(frames);
        }
	}

	public enum StateMaterial {
	    TRANSPARENT,
	    COVER,
	    FILLED
	}

	private class StatePresenter {
		private readonly List<GameObject> ballObjects = new List<GameObject>();
		private readonly Configuration config;
		private readonly Material transparent;
		private readonly Material background;
		private readonly Dictionary<string, Material> mappedMaterials;
		private StateMaterial material;

		public StatePresenter(Material transparent, Material background, Dictionary<string, Material> mappedMaterials, Configuration config) {
			this.config = config;
			this.transparent = transparent;
			this.background = background;
			this.mappedMaterials = mappedMaterials;
			this.material = StateMaterial.COVER;
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

            scaleBall(ballObject);

			updateLocationText(ball.id, ball.type, ball.trackingCount, ballObject);
		}

		private void append() {
			var ballObject = GameObject.CreatePrimitive(PrimitiveType.Sphere);
			ballObject.GetComponent<MeshRenderer>().material = transparent;
			ballObject.AddComponent<BallObjectInformation>();
			applyMaterial(ballObject, material);

			GameObject textObject = new GameObject("Text");
			TMPro.TextMeshPro textMesh = textObject.AddComponent<TMPro.TextMeshPro>();
			textMesh.fontSize = 4;
			textMesh.GetComponent<MeshRenderer >().enabled = false;
			textObject.transform.SetParent(ballObject.transform, false);
			textObject.transform.localPosition = new Vector3(0, -2, 0);
			textObject.GetComponent<RectTransform>().sizeDelta = new Vector2(5, 2);

			ballObjects.Add(ballObject);
		}

        public void toggleMaterial() {
			switch(this.material) {
			    case StateMaterial.TRANSPARENT:
                    setMaterial(StateMaterial.COVER);
                    break;
			    case StateMaterial.COVER:
			        setMaterial(StateMaterial.FILLED);
                    break;
                case StateMaterial.FILLED:
                    setMaterial(StateMaterial.TRANSPARENT);
                    break;
			}
		}

		public void setMaterial(StateMaterial material) {
		    this.material = material;
		    applyMaterial(material);
		}

		private void applyMaterial(StateMaterial material) {
		    foreach(var ball in ballObjects) {
                applyMaterial(ball, material);
		    }
		}

		private void applyMaterial(GameObject ballObject, StateMaterial material) {
            switch(material) {
                case StateMaterial.TRANSPARENT:
                    ballObject.GetComponent<MeshRenderer>().material = transparent;
                    break;
                case StateMaterial.COVER:
                    ballObject.GetComponent<MeshRenderer>().material = background;
                    break;
                case StateMaterial.FILLED:
                    ballObject.GetComponent<MeshRenderer>().material = getMaterialForBall(ballObject);
                    break;
            }
            scaleBall(ballObject);
        }

		public void enableDebug(bool debug) {
			foreach(var ball in ballObjects) {
            	var textObject = ball.transform.Find("Text").gameObject;
            	textObject.GetComponent<TMPro.TextMeshPro>().GetComponent<MeshRenderer>().enabled = debug;
            }
		}

		public void showHalo(bool show) {
			if (show) {
				Utility.drawCircle(ballObjects, 0.75f, 50);
			} else {
				foreach (var ball in ballObjects) {
					Destroy(ball.GetComponent<LineRenderer>());
				}
			}
		}

		private Material getMaterialForBall(GameObject ballObject) {
            BallObjectInformation ballInfo = ballObject.GetComponent<BallObjectInformation>();
            return mappedMaterials[ballInfo.type];
        }

        private void scaleBall(GameObject ballObject) {
            float radius = config.radius;
            ballObject.transform.localScale = new Vector3(radius, radius, radius) * 2 * StretchingUtility.get().scale;

            if (material == StateMaterial.COVER) {
                // In order to cover ball display from animator
                ballObject.transform.localScale *= 1.25f;
            }
        }

		private void updateLocationText(string id, string type, int trackingCount, GameObject ball) {
			var textObject = ball.transform.Find("Text").gameObject;
			var pos = new Vec2{x = ball.transform.position.x, y = ball.transform.position.y};
			var invPos = StretchingUtility.get().invPosition(pos);
			textObject.GetComponent<TMPro.TextMeshPro>().SetText(string.Format("[{0:F4}; {1:F4}]\n{2} {3}", invPos.x, invPos.y, id, trackingCount));
		}
	}

    public enum BallMaterial {
        CIRCLE,
        FILLED,
        INVISIBLE
    }
	
	private class AnimatorState {
		
		private static readonly double WAIT_BEFORE_BREAK = 2.0f; // seconds
		private static readonly double WAIT_AFTER_BREAK = 2.0f; // seconds
		
		private readonly Configuration config;
		private readonly Dictionary<string, GameObject> ballObjects = new Dictionary<string, GameObject>();
		private readonly List<GameObject> lines = new List<GameObject>();
		private readonly GameObject queue;
		private readonly Dictionary<string, Material> mappedMaterials;
		private readonly Material transparent;
		private readonly Material dottedLineMaterial;
		
		private KeyFrame[] frames;
		private State currentState;
		private int startFrameIndex;
		private int currentAnimationWindow;
		private BallMaterial material = BallMaterial.CIRCLE;
		private double globalTime;
		private bool repeatFirstBreak;
		
		public AnimatorState(Dictionary<string, Material> mappedMaterials, Material transparent,
		    Configuration config, GameObject queue, Material dottedLineMaterial) {
		    this.config = config;
			this.queue = queue;
			this.mappedMaterials = mappedMaterials;
			this.transparent = transparent;
			this.dottedLineMaterial = dottedLineMaterial;
		}

		public void setAnimation(KeyFrame[] frames) {
			this.frames = frames;
            float scale = StretchingUtility.get().scale;

            destroyBallObjects();

            if (frames.Length > 0) {
                foreach (var ball in frames[0].balls) {
                    var ballObject = GameObject.CreatePrimitive(PrimitiveType.Sphere);
                    ballObject.GetComponent<MeshRenderer>().material = transparent;
                    BallObjectInformation ballInfo = ballObject.AddComponent<BallObjectInformation>();
                    ballInfo.id = ball.id;
                    ballInfo.type = ball.type;
                    ballInfo.selectable = false;
                    ballObject.transform.position = StretchingUtility.get().position(convert(ball.position, -0.02f));
                    float radius = config.radius;
                    ballObject.transform.localScale = new Vector3(radius, radius, radius) * 2 * scale;

                    float circleLineWidth = Utility.LINE_WIDTH;
                    float borderLineWidth = Utility.LINE_WIDTH * 1.5f;
                    float circleRadius = (radius * scale) / (radius * scale * 2) - circleLineWidth * 2; // In local coordinates
                    GameObject lineBorder = new GameObject();
                    lineBorder.transform.parent = ballObject.transform;
                    lineBorder.transform.localPosition = new Vector3(0, 0, 0.01f);
                    lineBorder.transform.localScale = new Vector3(1, 1, 1);
                    Utility.drawCircle(new List<GameObject> { lineBorder }, circleRadius, 50, borderLineWidth);
                    Utility.drawCircle(new List<GameObject> { ballObject }, circleRadius, 50, circleLineWidth);

                    var circleRenderer = ballObject.GetComponent<LineRenderer>();
                    circleRenderer.material = mappedMaterials[ball.type];
                    circleRenderer.useWorldSpace = false;
                    ballObjects.Add(ball.id, ballObject);
                }
                applyMaterial(this.material);
            }

            reset();
		}
		
		private void setState(State state) {
			Debug.Log("Change from " + (currentState != null ? currentState.name() : "") + " to " + state.name());
			currentState = state;
			currentState.init();
		}
		
		public void reset() {
            queue.SetActive(false);
			this.startFrameIndex = 0;
			this.currentAnimationWindow = 0;
			this.globalTime = 0;
			this.repeatFirstBreak = false;
			
			if (this.frames.Length > 0) {
                KeyFrame start = this.frames[this.startFrameIndex];
                KeyFrame end = this.frames[this.startFrameIndex + 1];
                updateBalls(start, end, 0);
				setState(new LineDrawState(this));
            }
		}
		
		public void update(double timeDelta) {
			if (currentState != null) {
				currentState.update(timeDelta);
			}
		}
		
		public void appendLine(GameObject lineObject) {
			lines.Add(lineObject);
		}
		
		public void toggleBalls() {
			switch(material) {
			    case BallMaterial.CIRCLE:
                    setMaterial(BallMaterial.FILLED);
                    break;
			    case BallMaterial.FILLED:
			        setMaterial(BallMaterial.INVISIBLE);
                    break;
			    case BallMaterial.INVISIBLE:
			        setMaterial(BallMaterial.CIRCLE);
                    break;
			}
		}

		public void setMaterial(BallMaterial material) {
		    this.material = material;
		    applyMaterial(material);
		}
		
		public void delete() {
		    destroyBallObjects();
		    destroyLines();
		}
		
		public bool isFinished() {
			return false;
		}
		
		private void destroyBallObjects() {
            foreach (var ballObject in ballObjects.Values) {
                Destroy(ballObject);
            }
            ballObjects.Clear();
        }

        private void destroyLines() {
            foreach (var line in lines) {
                Destroy(line);
            }
            lines.Clear();
        }

		private void applyMaterial(BallMaterial material) {
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
		
		private Ball findCueBall(Ball[] balls) {
		    foreach (var ball in balls) {
                if (ball.type == "WHITE" && (ball.velocity.x != 0.0 || ball.velocity.y != 0.0)) {
                    return ball;
                }
            }
            return null;
		}
		
		private Ball findBall(string id, KeyFrame frame) {
			foreach (var ball in frame.balls) {
				if (ball.id == id) {
					return ball;
				}
			}
			return null;
		}
		
		private void updateBalls(KeyFrame start, KeyFrame end, double time) {
			double duration = end.time - start.time;
			double timeDeltaInAnimationWindow = time - start.time;

			var unvisited = ballObjects.Keys.ToList();
			foreach (var startBall in start.balls) {
				var ballObject = ballObjects[startBall.id];
				if (startBall.visible) {
					var endBall = findBall(startBall.id, end);

					if (material != BallMaterial.INVISIBLE) {
						ballObject.SetActive(true);
					}
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
			
			if (timeDeltaInAnimationWindow == duration) {
				foreach (var endBall in end.balls) {
					var ballObject = ballObjects[endBall.id];
					if (!endBall.visible) {
						ballObject.SetActive(false);
					}
				}
			}
		}
			
		private void updatePosition(Ball start, Ball end, GameObject gameObject, double duration, double timeDelta) {
			Vector3 startVelocity = convert(start.velocity,0.0f);
			Vector3 endVelocity = convert(end.velocity,0.0f);
			Vector3 a = duration == 0.0f ? new Vector3(0.0f, 0.0f, 0.0f) : (endVelocity - startVelocity) * (float)(1/duration);
			gameObject.transform.position = StretchingUtility.get().position(0.5f * a * (float)(timeDelta * timeDelta) + startVelocity * (float)timeDelta + convert(start.position, -0.02f));
		}
		
		private abstract class State {
			protected readonly AnimatorState machine;
			
			public State(AnimatorState machine) {
				this.machine = machine;
			}
			
			public virtual void init() {
				update(0);
			}
			
			public abstract string name();
			
			public abstract void update(double timeDelta);
		}
		
		private class LineDrawState : State {
			public LineDrawState(AnimatorState machine) : base(machine) {}
			
			public override void init() {
				machine.updateBalls(machine.frames[machine.startFrameIndex], machine.frames[machine.startFrameIndex+1], machine.globalTime);
				drawLines(machine.currentAnimationWindow);
				base.init();
			}
			
			public override string name() {
				return "LineDrawState";
			}
			
			public override void update(double timeDelta) {
				machine.setState(new WaitStateBeforeBrake(machine));
			}
			
			private void drawLines(int currentAnimationWindow) {
				machine.destroyLines();

				int windowCount = -1;
				List<KeyFrame> keyFrames = new List<KeyFrame>();
				for (int i = 0; i <= machine.frames.Length - 1; i++) {
					var frame = machine.frames[i];
					if (frame.firstFrame) {
						windowCount++;
					}

					if (windowCount == currentAnimationWindow) {
						keyFrames.Add(frame);
					} else if (windowCount > currentAnimationWindow) {
						break;
					}
				}
				
				HashSet<string> impulsIds = new HashSet<string>(); 
				for (int i = keyFrames.Count - 1; i > 0; i-=2) {
					var end = keyFrames[i];
					var start = keyFrames[i-1];

					HashSet<string> alreadyHandled = new HashSet<string>();
					foreach (var endBall in end.balls) {
						var startBall = machine.findBall(endBall.id, start);
						if (endBall.events.eventType == EventType.BALL_POTTING) {
							impulsIds.Add(endBall.id);
						} else if (impulsIds.Contains(endBall.id) && endBall.events.eventType == EventType.BALL_COLLISION && startBall.events.eventType == EventType.BALL_IN_REST) {
							var involvedBallId = endBall.events.involvedBallId1 != endBall.id ? endBall.events.involvedBallId1 : endBall.events.involvedBallId2;
							if (!alreadyHandled.Contains(involvedBallId)) {
								
								if (impulsIds.Contains(endBall.id) && impulsIds.Contains(involvedBallId)) {
									continue;
								}
								
								impulsIds.Remove(endBall.id);
								impulsIds.Add(involvedBallId);
								
								alreadyHandled.Add(involvedBallId);
								alreadyHandled.Add(endBall.id);
							}
						}
					}

					foreach (var startBall in start.balls) {
						var endBall = machine.findBall(startBall.id, end);
						if (endBall != null && startBall.position != endBall.position) {
							GameObject lineObject = new GameObject(string.Format("Line{0}", startBall.id));
							LineRenderer lRend = lineObject.AddComponent<LineRenderer>();
								
							if (impulsIds.Contains(startBall.id)) {
								strikeThrough(ref lRend);
							} else {
								dotted(ref lRend);
							}
								
							lRend.startWidth = 0.02f;
							lRend.endWidth = 0.02f;
							lRend.SetPosition(0, StretchingUtility.get().position(convert(startBall.position, -0.01f)));
							lRend.SetPosition(1, StretchingUtility.get().position(convert(endBall.position, -0.01f)));
							machine.appendLine(lineObject);
						}
					}
				}
			}
			
			private void strikeThrough(ref LineRenderer renderer) {
				renderer.material = new Material(Shader.Find("Hidden/Internal-Colored"));
				renderer.material.color = Color.white;
			}
			
			private void dotted(ref LineRenderer renderer) {
				renderer.material = machine.dottedLineMaterial;
				renderer.textureMode = LineTextureMode.Tile;
				renderer.material.SetTextureScale("_MainTex", new Vector2(10, 10));
			}
		}
		
		private class WaitStateBeforeBrake : State {
			
			private double waitTime = AnimatorState.WAIT_BEFORE_BREAK;
			
			public WaitStateBeforeBrake(AnimatorState machine) : base(machine) {}
			
			public override string name() {
				return "WaitStateBeforeBrake";
			}
			
			public override void update(double timeDelta) {
				if (timeDelta < 0) {
					machine.setState(new BreakState(machine)); // Zurückspulen
					return;
				}
				
				waitTime -= timeDelta;
				if (waitTime <= 0) {
					machine.setState(new QueueState(machine));
				}
			}
		}
		
		private class QueueState : State {
			
			private static readonly float QUEUE_DIST = 0.1f; // 10 cm
			private static readonly float QUEUE_DISPLAY_TIME = 1f; // 2s
			
			private double endTime;
			private KeyFrame startFrame;
			private double currentTime = 0;
			
			public QueueState(AnimatorState machine) : base(machine) {}
			
			public override string name() {
				return "QueueState";
			}
			
			public override void init() {
				startFrame = findStartFrame(machine.currentAnimationWindow);
				endTime = calculateQueueEndTime(startFrame);
				machine.queue.SetActive(true);
				base.init();
			}
			
			public override void update(double timeDelta) {
				
				currentTime = Math.Min(currentTime + timeDelta, this.endTime);
				doAnimateQueue(currentTime, endTime, startFrame);
				
				if (currentTime >= endTime) {
					machine.setState(new BreakState(machine));
				}
			}
			
			private KeyFrame findStartFrame(int currentAnimationWindow) {
				
				int windowCount = -1;
				for (int i = 0; i < machine.frames.Length - 1; i+=2) {
					var start = machine.frames[i];
					if (start.firstFrame) {
						windowCount++;
						
						if (windowCount == currentAnimationWindow) {
							return start;
						}
					}
				}
				return null;
			}
			
			private double calculateQueueEndTime(KeyFrame start) {
				Ball whiteBall = machine.findCueBall(start.balls);
				if (whiteBall == null) {
					return 0.0;
				}

				var endSpeed = convert(whiteBall.velocity, 0);
				var direction = Vector3.Normalize(endSpeed);
				float scale = StretchingUtility.get().scale;
				float radius = (machine.ballObjects[whiteBall.id].transform.localScale.x / scale) / 2;

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
				Ball whiteBall = machine.findCueBall(start.balls);
				if (whiteBall == null) {
					return false;
				}

				var endSpeed = convert(whiteBall.velocity, 0);
				var direction = Vector3.Normalize(endSpeed);
				float scale = StretchingUtility.get().scale;
				float radius = (machine.ballObjects[whiteBall.id].transform.localScale.x / scale) / 2;

				var startDirection = direction * (QUEUE_DIST + radius);
				var startPos = convert(whiteBall.position, 0) - startDirection;

				var a = endSpeed / (float)totalTime;
				time = Math.Min(time, totalTime);
				var currentPos = a / 2 * (float)(time * time) + startPos;

				float degrees = (float)(180 / Math.PI) * (float) Math.Acos(Vector3.Dot(new Vector3(0, -1, 0), endSpeed) / (endSpeed.magnitude));
				machine.queue.transform.eulerAngles = new Vector3(0, 0, endSpeed.x > 0 ? degrees : -degrees);

				float lengthToMove = machine.queue.transform.localScale.y;
				var toMove = Vector3.Normalize(endSpeed) * lengthToMove;
				machine.queue.transform.position = StretchingUtility.get().position(currentPos) - toMove;

				bool isFinished = time == totalTime;

				return !isFinished;
			}
		}
		
		private class BreakState : State {
			
			private readonly double QUEUE_DISPLAY_TIME = 1.5;
			
			private double localTime = 0;
			
			public BreakState(AnimatorState machine) : base(machine) {}
			
			public override string name() {
				return "BreakState";
			}
			
			public override void update(double timeDelta) {
				
				localTime += timeDelta;
				if (localTime >= QUEUE_DISPLAY_TIME) {
					machine.queue.SetActive(false);
				}
				
				machine.globalTime = machine.globalTime + timeDelta;
				double endTime = machine.frames[machine.frames.Length - 1].time;
				if (machine.startFrameIndex == 0) {
					machine.globalTime = Math.Max(0, machine.globalTime);
				} else if (machine.startFrameIndex == machine.frames.Length - 2) {
					machine.globalTime = Math.Min(endTime, machine.globalTime);
				}
				var change = mayUpdateKeyFrames(machine.globalTime, machine.frames);
				
				if (change == Change.NONE) {
					machine.updateBalls(machine.frames[machine.startFrameIndex], machine.frames[machine.startFrameIndex+1], machine.globalTime);
				} else {
					if (change == Change.FORWARD) {
						machine.updateBalls(machine.frames[machine.startFrameIndex-2], machine.frames[machine.startFrameIndex-1], machine.frames[machine.startFrameIndex-1].time);
					} else if (change == Change.BACKWARD) {
						machine.updateBalls(machine.frames[machine.startFrameIndex+2], machine.frames[machine.startFrameIndex+3], machine.frames[machine.startFrameIndex+2].time);
					}
					machine.queue.SetActive(false);
					machine.globalTime = machine.frames[machine.startFrameIndex].time;
					
					if (machine.repeatFirstBreak) {
						machine.setState(new BreakWaitRepeatState(machine));
					} else {
						machine.setState(new BreakWaitState(machine));
					}
				}
				
				if (machine.globalTime >= endTime) {
					machine.setState(new BreakWaitRepeatState(machine)); // Repeat first state
				}
			}
			
			private enum Change {
				NONE,
				FORWARD,
				BACKWARD
			}
			
			private Change mayUpdateKeyFrames(double time, KeyFrame[] frames) {
				KeyFrame currentEndFrame = frames[machine.startFrameIndex + 1];
				KeyFrame currentStartFrame = frames[machine.startFrameIndex];
				if (currentEndFrame.time < time) {
					machine.startFrameIndex += 2;
					
					if (frames[machine.startFrameIndex].firstFrame) {
						machine.currentAnimationWindow++;
						return Change.FORWARD;
					}
				} else if (currentStartFrame.time > time && time >= 0) {
					machine.startFrameIndex -= 2;
					if (machine.startFrameIndex < frames.Length - 1) {
						if (frames[machine.startFrameIndex + 2].firstFrame) {
							machine.currentAnimationWindow--;
							return Change.BACKWARD;
						}
					}
				}
				return Change.NONE;
			}
		}
		
		private class BreakWaitState : State {
			
			private double waitTime = AnimatorState.WAIT_AFTER_BREAK;
			
			public BreakWaitState(AnimatorState machine) : base(machine) {}
			
			public override string name() {
				return "BreakWaitState";
			}
			
			public override void update(double timeDelta) {
				if (timeDelta < 0) {
					machine.setState(new LineDrawState(machine)); // Zurückspulen
					return;
				}
				
				waitTime -= timeDelta;
				if (waitTime <= 0) {
					machine.setState(new LineDrawState(machine));
				}
			}
		}
		
		private class BreakWaitRepeatState : State {
			private double waitTime = AnimatorState.WAIT_AFTER_BREAK;
			
			public BreakWaitRepeatState(AnimatorState machine) : base(machine) {}
			
			public override string name() {
				return "BreakWaitResetState";
			}
			
			public override void update(double timeDelta) {
				if (timeDelta < 0) {
					machine.setState(new LineDrawState(machine)); // Zurückspulen
					return;
				}
				
				waitTime -= timeDelta;
				if (waitTime <= 0) {
					machine.repeatFirstBreak = true;
					machine.globalTime = 0;
					machine.startFrameIndex = 0;
					machine.currentAnimationWindow = 0;
					KeyFrame start = machine.frames[machine.startFrameIndex];
					KeyFrame end = machine.frames[machine.startFrameIndex + 1];
					machine.setState(new LineDrawState(machine));
				}
			}
			
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
