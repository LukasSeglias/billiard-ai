using System.Threading;
using System;
using UnityEngine;
using System.Collections.Generic;
using System.Net;
using System.Text;
using System.IO;
using System.Runtime.InteropServices;

public delegate void AnimationEvent (object sender, RootObject e);

public class AnimationService : MonoBehaviour
{
	////////////////////////////////////////////////////////////////////
	// Public Interface
	////////////////////////////////////////////////////////////////////
	public static event Action<RootObject> OnAnimationReceived;
	public static event Action OnCaptureState;
	
	void Start() {
		onStart();
		debugger((message) => Debug.Log(message));
			
		// Register native events
		onAnimationChangedEvent((rootObject) => OnAnimationReceived?.Invoke(map(rootObject)));
	}
	
	void OnApplicationQuit() {
		Debug.Log("Quit");
		onTearDown();
	}
	
	void FixedUpdate() {
		processEvents();
	}
	
	public static void setConfiguration(Configuration config) {
		configuration(map(config));
	}
	
	public static void setState(List<GameObject> balls, StretchingBehaviour heightBehaviour, StretchingBehaviour widthBehaviour) {
		BallState_t[] ballStates = map(balls, heightBehaviour, widthBehaviour);
		BilliardState_t billiardState = new BilliardState_t(ballStates);
		billiardState.ballSize = ballStates.Length;
		state(billiardState);
	}
	
	public static void captureState() {
		OnCaptureState?.Invoke();
		capture();
	}
	
	public static void searchSolution(Search value) {
		search(map(value));
	}
	
	////////////////////////////////////////////////////////////////////
	// Native Interface Declaration
	////////////////////////////////////////////////////////////////////
	
	[UnmanagedFunctionPointer(CallingConvention.StdCall)]
	private delegate void AnimationChangedEventCallback(RootObject_t rootObject);
	[UnmanagedFunctionPointer(CallingConvention.StdCall)]
	private delegate void Debugger(string message);
	
	[DllImport("unity_adapter", CallingConvention = CallingConvention.Cdecl)]
	private static extern void onStart();
	[DllImport("unity_adapter", CallingConvention = CallingConvention.Cdecl)]
	private static extern void onTearDown();
	[DllImport("unity_adapter", CallingConvention = CallingConvention.Cdecl)]
	private static extern void processEvents();
	
	[DllImport("unity_adapter", CallingConvention = CallingConvention.Cdecl)]
	private static extern void onAnimationChangedEvent([MarshalAs(UnmanagedType.FunctionPtr)] AnimationChangedEventCallback callback);
	
	[DllImport("unity_adapter", CallingConvention = CallingConvention.Cdecl)]
	private static extern void configuration(Configuration_t config);
	
	[DllImport("unity_adapter", CallingConvention = CallingConvention.Cdecl)]
	private static extern void capture();
	
	[DllImport("unity_adapter", CallingConvention = CallingConvention.Cdecl)]
	private static extern void search(Search_t search);
	
	[DllImport("unity_adapter", CallingConvention = CallingConvention.Cdecl)]
	private static extern void debugger([MarshalAs(UnmanagedType.FunctionPtr)] Debugger callback);
	
	[DllImport("unity_adapter", CallingConvention = CallingConvention.Cdecl)]
	private static extern void state(BilliardState_t state);
	
	////////////////////////////////////////////////////////////////////
	// Adapters for native interaction
	////////////////////////////////////////////////////////////////////
	
	[StructLayout(LayoutKind.Sequential)]
	struct RootObject_t {
		public IntPtr animations;
		public int animationSize;
	}
	
	[StructLayout(LayoutKind.Sequential)]
	struct AnimationModel_t {
		public IntPtr keyFrames;
		public int keyFrameSize;
	}
	
	[StructLayout(LayoutKind.Sequential)]
	struct KeyFrame_t
	{
		public double time;
		public IntPtr balls;
		public int ballSize;
	}
	
	[StructLayout(LayoutKind.Sequential)]
	struct Vec2_t {
		public double x;
		public double y;
	}

	[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
	struct Ball_t
	{
		public string type;
		public string id;
		public Vec2_t position;
		public Vec2_t velocity;
		[MarshalAs(UnmanagedType.I1)] public bool visible;
	}
	
	[StructLayout(LayoutKind.Sequential)]
	struct Configuration_t {
		public float radius;
		public float width;
		public float height;
	}
	
	[StructLayout(LayoutKind.Sequential)]
	class BilliardState_t {
		public BilliardState_t(BallState_t[] ballStates) {
			int size = 0;
			foreach(var ballState in ballStates) {
				size += Marshal.SizeOf(ballState);
			}
            balls = Marshal.AllocHGlobal(size);
			
			long currentPtr = balls.ToInt64();
			for (int i = 0; i < ballStates.Length; i++)
			{
				IntPtr ptr = new IntPtr(currentPtr);
				Marshal.StructureToPtr(ballStates[i], ptr, false);
				currentPtr += Marshal.SizeOf(ballStates[i]);
			}
		}
		
		~BilliardState_t() {
			Marshal.FreeHGlobal(balls);
		}
		
		public IntPtr balls;
		public int ballSize;
	}

	[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
	class BallState_t {
		public string type;
		public string id;
		public Vec2_t position;
	}
	
	[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
	class Search_t {
		public string id;
		public string type;
	}
	
	////////////////////////////////////////////////////////////////////
	// Mapping between adapter and application model
	////////////////////////////////////////////////////////////////////
	
	private static Configuration_t map(Configuration from) {
		return new Configuration_t{radius = from.radius, width = from.width, height = from.height};
	}
	
	private static RootObject map(RootObject_t from) {
		RootObject result = new RootObject();
		result.animations = new AnimationModel[from.animationSize];
		
		AnimationModel_t[] animations = GetNativeArray<AnimationModel_t>(from.animations, from.animationSize);
		
		for(int i = 0; i < from.animationSize; i++) {
			result.animations[i] = map(animations[i]);
		}
		
		return result;
	}
	
	private static AnimationModel map(AnimationModel_t from) {
		AnimationModel result = new AnimationModel();
		result.keyFrames = new KeyFrame[from.keyFrameSize];
		KeyFrame_t[] keyFrames = GetNativeArray<KeyFrame_t>(from.keyFrames, from.keyFrameSize);
		
		for(int i = 0; i < from.keyFrameSize; i++) {
			result.keyFrames[i] = map(keyFrames[i]);
		}
			
		return result;
	}
	
	private static KeyFrame map(KeyFrame_t from) {
		KeyFrame result = new KeyFrame();
		result.balls = new Ball[from.ballSize];
		result.time = from.time;
		Ball_t[] balls = GetNativeArray<Ball_t>(from.balls, from.ballSize);
		
		for(int i = 0; i < from.ballSize; i++) {
			result.balls[i] = map(balls[i]);
		}
			
		return result;
	}
	
	private static Ball map(Ball_t from) {
		Ball result = new Ball();
		result.type = from.type;
		result.id = from.id;
		result.position = map(from.position);
		result.velocity = map(from.velocity);
		result.visible = from.visible;
			
		return result;
	}
	
	private static Search_t map(Search search) {
		return new Search_t{id = search.id, type = search.type};
	}
	
	private static Vec2 map(Vec2_t from) {
		return new Vec2 {x = from.x, y = from.y};
	}
	
	private static BallState_t[] map(List<GameObject> balls, StretchingBehaviour heightBehaviour, StretchingBehaviour widthBehaviour) {
		BallState_t[] ballStates = new BallState_t[balls.Count];
		
		for (int i = 0; i < balls.Count; i++) {
			var ball = balls[i];
			var pickableInfo = ball.GetComponent<PickableInformation>();
			BallState_t ballState = new BallState_t {
				type = pickableInfo.type,
				id = pickableInfo.id,
				position = new Vec2_t {
					x = widthBehaviour.inverse(ball.transform.position.x),
					y = heightBehaviour.inverse(ball.transform.position.y)
				}
			};
			ballStates[i] = ballState;
		}
		
		
		return ballStates;
	}
	
	////////////////////////////////////////////////////////////////////
	// Helpers
	////////////////////////////////////////////////////////////////////
	
	// https://answers.unity.com/questions/34606/how-do-i-pass-arrays-from-c-to-c-in-unity-if-at-al.html
	private static T[] GetNativeArray<T>(IntPtr array, int length) {
         T[] result = new T[length];
         int size = Marshal.SizeOf (typeof(T));
 
         if (IntPtr.Size == 4) {
             // 32-bit system
             for (int i = 0; i < result.Length; i++) {
                 result [i] = (T)Marshal.PtrToStructure (array, typeof(T));
                 array = new IntPtr (array.ToInt32 () + size);
             }
         } else {
             // probably 64-bit system
             for (int i = 0; i < result.Length; i++) {
                 result [i] = (T)Marshal.PtrToStructure (array, typeof(T));
                 array = new IntPtr (array.ToInt64 () + size);
             }
         }
         return result;
     }
}
