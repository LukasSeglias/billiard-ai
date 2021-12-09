using System;
using System.Collections.Generic;
using UnityEngine;
using System.Linq;

[Serializable]
public class RootObject
{
	public AnimationModel[] animations = new AnimationModel[0];
}

[Serializable]
public class AnimationModel
{
    public KeyFrame[] keyFrames;
}

[Serializable]
public class KeyFrame
{
    public double time;
    public Ball[] balls;
	public bool firstFrame;
}

[Serializable]
public class Vec2 {
	public double x;
	public double y;
	
	public static Vec2 operator /(Vec2 vec, float value) => new Vec2{x = vec.x / value, y = vec.y / value};
	public static Vec2 operator *(Vec2 vec, double value) => new Vec2{x = vec.x * value, y = vec.y * value};
	public static Vec2 operator *(double value, Vec2 vec) => new Vec2{x = vec.x * value, y = vec.y * value};
	public static Vec2 operator -(Vec2 v1, Vec2 v2) => new Vec2{ x = v1.x - v2.x, y = v1.y - v2.y };
    public static Vec2 operator +(Vec2 v1, Vec2 v2) => new Vec2{ x = v1.x + v2.x, y = v1.y + v2.y };
	public static double dot(Vec2 v1, Vec2 v2) {
	    return v1.x * v2.x + v1.y * v2.y;
	}
	public double length() {
	    return Math.Sqrt(x*x + y*y);
	}
	public Vec2 normalized() {
	    return this * (1.0 / length());
	}
	public Vec2 perp() {
	    return new Vec2 { x = -y, y = x };
	}
}

[Serializable]
public class Vec3 {
	public double x;
	public double y;
	public double z;

	public static Vec3 operator /(Vec3 vec, float value) => new Vec3{
	    x = vec.x / value,
        y = vec.y / value,
        z = vec.z / value
	};
}

[Serializable]
public enum EventType {
	BALL_MOVING,
    BALL_COLLISION,
    BALL_RAIL_COLLISION,
    BALL_POTTING,
    BALL_SHOT,
    BALL_IN_REST
}

[Serializable]
public class Event {
	public EventType eventType;
	public string involvedBallId1;
	public string involvedBallId2;
}

[Serializable]
public class Ball
{
    public string type;
	public string id;
    public Vec2 position;
    public Vec2 velocity;
	public bool visible;
	public Event events;
}

[Serializable]
public class RailSegment {
	public Vec2 start;
	public Vec2 end;
	public Vec2 shiftDirection;
	public string id;

	public static RailSegment operator /(RailSegment segment, float value) => new RailSegment{
		id = segment.id,
		start = segment.start / value,
		end = segment.end / value,
		shiftDirection = segment.shiftDirection
	};

	public Vec2 normal() {
	    Vec2 delta = (end - start).normalized();
        return delta.perp();
	}
}

public enum PocketType {
    CORNER,
	CENTER
}
	
[Serializable]
public class Circle {
    public string id;
	public float radius;
	public Vec2 position;
	public Vec2 normal;
	public PocketType pocketType;
	public Vec2[] pottingPoints;
	
	public static Circle operator /(Circle circle, float value) => new Circle{
	    id = circle.id,
		radius = circle.radius / value,
		position = circle.position / value,
		normal = circle.normal,
		pocketType = circle.pocketType,
		pottingPoints = circle.pottingPoints.Select(pottingPoint => pottingPoint / value).ToArray(),
	};
}
	
[Serializable]
public class ArucoMarkers {

    public int patternSize;
    public float sideLength;
	public Vec3 m0;
	public Vec3 m1;
	public Vec3 m2;
	public Vec3 m3;
	
	public static ArucoMarkers operator /(ArucoMarkers markers, float value) => new ArucoMarkers{
		patternSize = markers.patternSize,
		sideLength = markers.sideLength,
		m0 = new Vec3 { x = markers.m0.x, y = markers.m0.y, z = markers.m0.z },
		m1 = new Vec3 { x = markers.m1.x, y = markers.m1.y, z = markers.m1.z },
		m2 = new Vec3 { x = markers.m2.x, y = markers.m2.y, z = markers.m2.z },
		m3 = new Vec3 { x = markers.m3.x, y = markers.m3.y, z = markers.m3.z },
	};
}

[Serializable]
public class CameraIntrinsics {
	public double fx;
	public double fy;

	public double cx;
	public double cy;

	public double skew;

	public double k1;
	public double k2;
	public double k3;

	public double p1;
	public double p2;

	public double sx;
	public double sy;
}

[Serializable]
public class Plane {

	public Vec3 point;
	public Vec3 normal;
}

[Serializable]
public class WorldToModel {

	public Vec3 translation;
}

[Serializable]
public class Table {

    public double innerTableLength;
    public double innerTableWidth;
    public double ballDiameter;
    public double arucoHeightAboveInnerTable;
    public double railWorldPointZComponent;
	public Vec3 worldToRail;
	public float minimalPocketVelocity;
}

[Serializable]
public class Spot {
	public string type;
	public Vec2 position;
}

[Serializable]
public class Configuration {
	public float radius;
	public float width;
    public float height;
	public float scale;
	public string headRail;
	public RailSegment[] segments;
	public Circle[] targets;
	public Spot[] spots;
	public ArucoMarkers markers;
	public CameraIntrinsics camera;
	public Plane ballPlane;
	public WorldToModel worldToModel;
	public Table table;
	public int solutions;
	public Correction correctionWidth;
	public Correction correctionHeight;
	public string[] infinityModeSearchTypes;
	public string[] coloredSearchTypes;

	public static Configuration operator /(Configuration config, float value) => new Configuration{
		radius = config.radius / value,
		width = config.width / value,
		height = config.height / value,
		scale = config.scale,
		segments = config.segments.Select(segment => segment / value).ToArray(),
		targets = config.targets.Select(target => target / value).ToArray(),
        markers = config.markers / value,
        ballPlane = new Plane {
            point  = new Vec3{ x = config.ballPlane.point.x,  y = config.ballPlane.point.y,  z = config.ballPlane.point.z },
            normal = new Vec3{ x = config.ballPlane.normal.x, y = config.ballPlane.normal.y, z = config.ballPlane.normal.z }
        },
        worldToModel = new WorldToModel {
            translation = new Vec3 { x = config.worldToModel.translation.x, y = config.worldToModel.translation.y, z = config.worldToModel.translation.z }
        },
        table = new Table {
            innerTableLength = config.table.innerTableLength,
            innerTableWidth = config.table.innerTableWidth,
            ballDiameter = config.table.ballDiameter,
            arucoHeightAboveInnerTable = config.table.arucoHeightAboveInnerTable,
            railWorldPointZComponent = config.table.railWorldPointZComponent,
            worldToRail = new Vec3 { x = config.table.worldToRail.x, y = config.table.worldToRail.y, z = config.table.worldToRail.z },
            minimalPocketVelocity = config.table.minimalPocketVelocity
        },
        solutions = config.solutions,
        camera = new CameraIntrinsics {fx = config.camera.fx, fy = config.camera.fy,
                           cx = config.camera.cx, cy = config.camera.cy,
                           skew = config.camera.skew,
                           k1 = config.camera.k1, k2 = config.camera.k2, k3 = config.camera.k3,
                           p1 = config.camera.p1, p2 = config.camera.p2,
                           sx = config.camera.sx, sy = config.camera.sy
        },
		correctionWidth = new Correction{factor = config.correctionWidth.factor, translation = config.correctionWidth.translation},
		correctionHeight = new Correction{factor = config.correctionHeight.factor, translation = config.correctionHeight.translation},
		infinityModeSearchTypes = config.infinityModeSearchTypes,
		coloredSearchTypes = config.coloredSearchTypes
	};
}

[Serializable]
public class Correction {
	public float factor;
	public float translation;
}

[Serializable]
public class Search
{
    public string id = "";
	public string[] types = new string[0];
}

public enum TableStatus {
    UNKNOWN,
    STABLE,
    UNSTABLE
}

[Serializable]
public class RootState {
	public BallState[] balls = new BallState[0];
	public TableStatus status = TableStatus.UNKNOWN;
}

[Serializable]
public class BallState {
	public string type;
	public string id;
    public Vec2 position;
    public int trackingCount;
}

public class BallObjectInformation : MonoBehaviour
{
    public string id;
	public string type;
	public bool selectable;
}