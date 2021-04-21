using System;
using System.Collections;
using System.Collections.Generic;
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
	public TextMeshPro infoText;
	public StretchingBehaviour HeightStretching;
	public StretchingBehaviour WidthStretching;
	public float Scale = 1.0f;
	
	private RootObject root = new RootObject();
	private Animator animator;
	private Dictionary<string, Material> mappedMaterials = new Dictionary<string, Material>();
	private bool isPlaying = true;
	private Configuration config;
    private int animationIndex = 0;
	
	// Start is called before the first frame update
    void Start()
    {
		SceneManager.SetActiveScene(SceneManager.GetSceneByName("MainScene"));
		
		foreach(var entry in Materials)
        {
            if (!mappedMaterials.ContainsKey(entry.key))
            {
                mappedMaterials.Add(entry.key, entry.material);
            }
        }
		
		AnimationService.OnAnimationReceived += animationChanged;
		AnimationService.OnCaptureState += disableInterface;
		
		config = ConfigurationLoader.load(ref HeightStretching, ref WidthStretching, ref Table);
		Scale = config.scale;
		
		Table.GetComponent<Renderer>().enabled = false;

		infoText.gameObject.SetActive(false);
    }
	
	void OnDestroy() {
		AnimationService.OnAnimationReceived -= animationChanged;
		AnimationService.OnCaptureState -= disableInterface;
	}

    // Update is called once per frame
    void Update()
    {
		AnimationModel[] animations = root.animations;
		
		if (Input.GetKey(KeyCode.UpArrow)) {
			animationIndex = (animationIndex + 1) % animations.Length;
			if (this.animator != null) {
				this.animator.delete();
				this.animator = null;
			}
			
		} else if (Input.GetKey(KeyCode.DownArrow)) {
			animationIndex = (animationIndex - 1) % animations.Length;
			if (this.animator != null) {
				this.animator.delete();
				this.animator = null;
			}
		}
		
		if (animator == null && animations.Length > 0) {
			animator = new Animator(root, animations[animationIndex].keyFrames, mappedMaterials, config, HeightStretching, WidthStretching, Scale);
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
			Table.GetComponent<Renderer>().enabled = !Table.GetComponent<Renderer>().enabled;
		} else if (Input.GetKeyDown(KeyCode.D)) {
			// TODO: Display position
		}
		
		if (isPlaying) {
			animate(Time.deltaTime);
		}
    }
	
	private void animate(double deltaTime) {		
		if (animator != null) {
			animator.update(deltaTime);
		}
	}
	
	private class Animator {
		
		private readonly KeyFrame[] frames;
		private readonly Dictionary<string, GameObject> ballObjects = new Dictionary<string, GameObject>();
		private readonly List<GameObject> lines = new List<GameObject>();
		private readonly double endTime;
		private readonly StretchingBehaviour heightStretching;
		private readonly StretchingBehaviour widthStretching;
		private int startFrameIndex;
		private double time;
		private bool activeByDefinition = true;
		
		public Animator(RootObject root, KeyFrame[] frames, Dictionary<string, Material> mappedMaterials, Configuration config,
			StretchingBehaviour heightStretching, StretchingBehaviour widthStretching, float scale) {
			this.frames = frames;
			this.heightStretching = heightStretching;
			this.widthStretching = widthStretching;
			foreach (var ball in frames[0].balls) {
				var ballObject = GameObject.CreatePrimitive(PrimitiveType.Sphere);
				ballObject.GetComponent<MeshRenderer>().material = mappedMaterials[ball.type];
				PickableInformation ballInfo = ballObject.AddComponent<PickableInformation>();
				ballInfo.id = ball.id;
				ballInfo.type = ball.type;
				ballObject.transform.position = position(convert(ball.position, 0));
				ballObject.transform.localScale = new Vector3((float) config.radius/0.5f * scale, (float) config.radius/0.5f * scale, (float) config.radius/0.5f * scale);
				ballObjects.Add(ball.id, ballObject);
			}
			this.endTime = frames[frames.Length - 1].time;
			
			reset();
			drawLines(mappedMaterials);
		}
		
		public void drawLines(Dictionary<string, Material> mappedMaterials) {
			float alpha = 1f;
			float alphaEnd = 0.2f;
			float alphaStep = (alpha-alphaEnd)/(this.frames.Length/2f);
			for (int i = 0; i < this.frames.Length - 1; i++) {
				var start = this.frames[i];
				var end = this.frames[i + 1];
				
				foreach (var startBall in start.balls) {
					var endBall = findBall(startBall.id, end);
					if (endBall != null && startBall.position != endBall.position) {
						GameObject lineObject = new GameObject(string.Format("Line{0}", startBall.id));
						LineRenderer lRend = lineObject.AddComponent<LineRenderer>();
						lRend.material = new Material(Shader.Find("Hidden/Internal-Colored"));
						Gradient gradient = new Gradient();
						gradient.SetKeys(
							new GradientColorKey[] { new GradientColorKey(mappedMaterials[startBall.type].color, 1.0f), new GradientColorKey(mappedMaterials[startBall.type].color, 1.0f) },
							new GradientAlphaKey[] { new GradientAlphaKey(alpha, alpha), new GradientAlphaKey(alpha, alpha) }
						);
						lRend.colorGradient = gradient;
						lRend.startWidth = 0.02f;
						lRend.endWidth = 0.02f;
						lRend.SetPosition(0, position(convert(startBall.position, 0)));
						lRend.SetPosition(1, position(convert(endBall.position, 0)));
						lRend.sortingOrder = 0;
						lines.Add(lineObject);
					}
				}
				alpha -= alphaStep;
			}
		}
		
		public void reset() {
			this.time = 0;
			this.startFrameIndex = 0;
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
			
			mayUpdateAnimationWindow(time, this.frames);
			if (!isFinished()) {
				KeyFrame start = this.frames[this.startFrameIndex];
				KeyFrame end = this.frames[this.startFrameIndex + 1];
				double duration = end.time - start.time;
				double timeDeltaInAnimationWindow = time - start.time;
					
				foreach (var startBall in start.balls) {
					var ballObject = ballObjects[startBall.id];
					if (startBall.visible) {
						var endBall = findBall(startBall.id, end);
						
						ballObject.SetActive(true);
						updatePosition(startBall, endBall, ballObject, duration, timeDeltaInAnimationWindow);
					} else {
						ballObject.SetActive(false);
					}
				}
			} else {
				KeyFrame end = this.frames[this.startFrameIndex + 1];
				foreach (var ball in end.balls) {
					var ballObject = ballObjects[ball.id];
					ballObject.SetActive(ball.visible);
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
		
		private Vector3 convert(Vec2 vector, float z) {
			return new Vector3((float)vector.x, (float)vector.y, z);
		}
		
		private Vector3 position(Vector3 pos) {
			return new Vector3(widthStretching.forward(pos.x), heightStretching.forward(pos.y), pos.z);
		}
		
		private void mayUpdateAnimationWindow(double time, KeyFrame[] frames) {
			KeyFrame currentEndFrame = frames[this.startFrameIndex + 1];
			KeyFrame currentStartFrame = frames[this.startFrameIndex];
			if (currentEndFrame.time < time) {
				this.startFrameIndex += 2;
			} else if (currentStartFrame.time > time && time >= 0) {
				this.startFrameIndex -= 2;
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
			Vector3 a = (endVelocity - startVelocity) * (float)(1/duration);
			gameObject.transform.position = position(0.5f * a * (float)(timeDelta * timeDelta) + startVelocity * (float)timeDelta + convert(start.position, 0));			
		}
	}
	
	private void animationChanged(RootObject root) {
		infoText.SetText("");
		infoText.gameObject.SetActive(false);
		enableInterface();
		this.root = root;
		
		if (this.animator != null) {
			this.animator.delete();
			this.animator = null;
		}
	}
	
	private void disableInterface() {
		Background.transform.position = new Vector3(0, 0, -0.8f);
	}
	
	private void enableInterface() {
		Background.transform.position = new Vector3(0, 0, 0.5f);
	}
}
