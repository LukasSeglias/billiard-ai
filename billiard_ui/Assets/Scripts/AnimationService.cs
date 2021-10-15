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
	public static event Action<RootState> OnStateReceived;
	
	private static bool doCapture;
	private static bool isLive;

	void Start() {		
		onStart();
		debugger((message) => Debug.Log(message));
			
		// Register native events
		onAnimationChangedEvent((rootObject) => {
			OnAnimationReceived?.Invoke(map(rootObject));
		});
		
		onStateChangedEvent((state) => {
			OnStateReceived?.Invoke(map(state));
		});
	}
	
	void OnApplicationQuit() {
		Debug.Log("Quit");
		onTearDown();
	}
	
	void Update() {
		if (doCapture) {
			capture();
			doCapture = false;
		} else if(isLive){
			doCapture = true;
		}

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

	public static void imageCapture() {
	    image();
	}
	
	public static void captureState(bool captureState) {
		isLive = captureState;
	}
	
	public static void captureState() {
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
	private delegate void StateChangedEventCallback(RootState_t rootState);
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
	private static extern void onStateChangedEvent([MarshalAs(UnmanagedType.FunctionPtr)] StateChangedEventCallback callback);
	
	[DllImport("unity_adapter", CallingConvention = CallingConvention.Cdecl)]
	private static extern void configuration(Configuration_t config);
	
	[DllImport("unity_adapter", CallingConvention = CallingConvention.Cdecl)]
	private static extern void capture();

	[DllImport("unity_adapter", CallingConvention = CallingConvention.Cdecl)]
    private static extern void image();
	
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
		[MarshalAs(UnmanagedType.I1)] public bool firstFrame;
	}
	
	[StructLayout(LayoutKind.Sequential)]
	struct Vec2_t {
		public double x;
		public double y;
	}

	[StructLayout(LayoutKind.Sequential)]
    struct Vec3_t {
        public double x;
        public double y;
        public double z;
    }
	
	[StructLayout(LayoutKind.Sequential)]
	public class RootState_t {
		public IntPtr balls;
		public int ballSize;
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
	struct RailSegment_t {
		public Vec2_t start;
		public Vec2_t end;
		public RailLocation location;
	}
	
	[StructLayout(LayoutKind.Sequential)]
	struct Circle_t {
		public float radius;
		public Vec2_t position;
		public PocketType pocketType;
	}
	
	[StructLayout(LayoutKind.Sequential)]
	struct ArucoMarkers_t {
	    public int patternSize;
		public float sideLength;
		public Vec3_t m0;
		public Vec3_t m1;
		public Vec3_t m2;
		public Vec3_t m3;
	}

	[StructLayout(LayoutKind.Sequential)]
    struct Plane_t {
        public Vec3_t point;
        public Vec3_t normal;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct WorldToModel_t {
        public Vec3_t translation;
    }

    [StructLayout(LayoutKind.Sequential)]
    class Table_t {

        public double innerTableLength;
        public double innerTableWidth;
        public double ballDiameter;
        public double arucoHeightAboveInnerTable;
        public double railWorldPointZComponent;
        public Vec3_t worldToRail;
        public float minimalPocketVelocity;
    }

	[StructLayout(LayoutKind.Sequential)]
    struct CameraIntrinsics_t {
        // Focal length in pixel
        public double fx;
        public double fy;

        // Principal point in pixel
        public double cx;
        public double cy;

        // Skew
        public double skew;

        // Radial distortion coefficients k1, k2, k3
        public double k1;
        public double k2;
        public double k3;

        // Tangential distortion coefficients p1, p2
        public double p1;
        public double p2;

        // Sensor pixel size in millimeters
        public double sx;
        public double sy;
    }

	[StructLayout(LayoutKind.Sequential)]
	class Configuration_t {
		public Configuration_t(float radius, float width, float height,
		RailSegment_t[] segments, Circle_t[] targets, ArucoMarkers_t markers, CameraIntrinsics_t camera, Plane_t ballPlane, WorldToModel_t worldToModel, Table_t table) {
			this.radius = radius;
			this.width = width;
			this.height = height;
			this.segments = allocate(segments);
			this.segmentSize = segments.Length;
			this.targets = allocate(targets);
			this.targetSize = targets.Length;
			this.markers = markers;
			this.camera = camera;
			this.ballPlane = ballPlane;
			this.worldToModel = worldToModel;
			this.table = table;
		}
		
		~Configuration_t() {
			Marshal.FreeHGlobal(segments);
			Marshal.FreeHGlobal(targets);
		}
		
		public float radius;
		public float width;
		public float height;
		public int segmentSize;
		public int targetSize;
		public IntPtr segments;
		public IntPtr targets;
		public ArucoMarkers_t markers;
		public CameraIntrinsics_t camera;
		public Plane_t ballPlane;
		public WorldToModel_t worldToModel;
		public Table_t table;
	}
	
	[StructLayout(LayoutKind.Sequential)]
	class BilliardState_t {
		public BilliardState_t(BallState_t[] ballStates) {
			this.balls = allocate(ballStates);
			this.ballSize = ballStates.Length;
			this.fromUnity = true;
		}
		
		~BilliardState_t() {
			Marshal.FreeHGlobal(balls);
		}
		
		public IntPtr balls;
		public int ballSize;
		[MarshalAs(UnmanagedType.I1)] public bool fromUnity;
	}

	[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
	class BallState_t {
		public string type;
		public string id;
		public Vec2_t position;
		[MarshalAs(UnmanagedType.I1)] public bool fromUnity = true;
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
		return new Configuration_t(from.radius, from.width, from.height, map(from.segments), map(from.targets), map(from.markers), map(from.camera), map(from.ballPlane), map(from.worldToModel), map(from.table));
	}
	
	private static RailSegment_t[] map(RailSegment[] from) {
		RailSegment_t[] segments = new RailSegment_t[from.Length];
		for (int i = 0; i < segments.Length; i++) {
			segments[i] = map(from[i]);
		}
		return segments;
	}
	
	private static RailSegment_t map(RailSegment from) {
		return new RailSegment_t{start = map(from.start), end = map(from.end), location = from.location};
	}
	
	private static Circle_t[] map(Circle[] from) {
		Circle_t[] circles = new Circle_t[from.Length];
		for (int i = 0; i < circles.Length; i++) {
			circles[i] = map(from[i]);
		}
		return circles;
	}
	
	private static Circle_t map(Circle from) {
		return new Circle_t{radius = from.radius, position = map(from.position), pocketType = from.pocketType};
	}

	private static ArucoMarkers_t map(ArucoMarkers from) {
		return new ArucoMarkers_t{
		    patternSize = from.patternSize,
		    sideLength = from.sideLength,
		    m0 = map(from.m0),
		    m1 = map(from.m1),
		    m2 = map(from.m2),
		    m3 = map(from.m3)
		};
	}

	private static Plane_t map(Plane from) {
        return new Plane_t{
            point = map(from.point),
            normal = map(from.normal)
        };
    }

    private static WorldToModel_t map(WorldToModel from) {
        return new WorldToModel_t{
            translation = map(from.translation)
        };
    }

    private static Table_t map(Table from) {
            return new Table_t{
                innerTableLength = from.innerTableLength,
                innerTableWidth = from.innerTableWidth,
                ballDiameter = from.ballDiameter,
                arucoHeightAboveInnerTable = from.arucoHeightAboveInnerTable,
                railWorldPointZComponent = from.railWorldPointZComponent,
                worldToRail = map(from.worldToRail),
                minimalPocketVelocity = from.minimalPocketVelocity
            };
        }

	private static CameraIntrinsics_t map(CameraIntrinsics from) {
    		return new CameraIntrinsics_t{fx = from.fx, fy = from.fy,
    		                    cx = from.cx, cy = from.cy,
    		                    skew = from.skew,
    		                    k1 = from.k1, k2 = from.k2, k3 = from.k3,
    		                    p1 = from.p1, p2 = from.p2,
    		                    sx = from.sx, sy = from.sy
    		                    };
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
		result.firstFrame = from.firstFrame;
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
		result.position = map(from.position) / 1000;
		result.velocity = map(from.velocity) / 1000;
		result.visible = from.visible;
			
		return result;
	}
	
	private static Search_t map(Search search) {
		return new Search_t{id = search.id, type = search.type};
	}
	
	private static Vec2 map(Vec2_t from) {
		return new Vec2 {x = from.x, y = from.y};
	}
	
	private static Vec2_t map(Vec2 from) {
		return new Vec2_t {x = from.x, y = from.y};
	}

	private static Vec3 map(Vec3_t from) {
        return new Vec3 {x = from.x, y = from.y, z = from.z};
    }

    private static Vec3_t map(Vec3 from) {
        return new Vec3_t {x = from.x, y = from.y, z = from.z};
    }
	
	private static BallState_t[] map(List<GameObject> balls, StretchingBehaviour heightBehaviour, StretchingBehaviour widthBehaviour) {
		BallState_t[] ballStates = new BallState_t[balls.Count];
		
		for (int i = 0; i < balls.Count; i++) {
			var ball = balls[i];
			var info = ball.GetComponent<BallObjectInformation>();
			BallState_t ballState = new BallState_t {
				type = info.type,
				id = info.id,
				position = new Vec2_t {
					x = widthBehaviour.inverse(ball.transform.position.x) * 1000,
					y = heightBehaviour.inverse(ball.transform.position.y) * 1000
				}
			};
			ballStates[i] = ballState;
		}
		
		
		return ballStates;
	}
	
	private static RootState map(RootState_t from) {
		RootState result = new RootState();
		result.balls = new BallState[from.ballSize];
		BallState_t[] balls = GetNativeArray<BallState_t>(from.balls, from.ballSize);
		
		for(int i = 0; i < from.ballSize; i++) {
			result.balls[i] = map(balls[i]);
		}
		
		return result;
	}
	
	private static BallState map(BallState_t from) {
		BallState result = new BallState();
		result.type = from.type;
		result.id = from.id;
		result.position = map(from.position) / 1000;
			
		return result;
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
	
	private static IntPtr allocate<T>(T[] models) {
		int size = 0;
		foreach(var model in models) {
			size += Marshal.SizeOf(model);
		}
        IntPtr pointer = Marshal.AllocHGlobal(size);
			
		long currentPtr = pointer.ToInt64();
		for (int i = 0; i < models.Length; i++)
		{
			IntPtr ptr = new IntPtr(currentPtr);
			Marshal.StructureToPtr(models[i], ptr, false);
			currentPtr += Marshal.SizeOf(models[i]);
		}
		return pointer;
	}
}
