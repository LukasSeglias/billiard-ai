﻿using System;
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
public class Ball
{
    public string type;
	public string id;
    public Vec2 position;
    public Vec2 velocity;
	public bool visible;
}

[Serializable]
public class RailSegment {
	public Vec2 start;
	public Vec2 end;
	
	public static RailSegment operator /(RailSegment segment, float value) => new RailSegment{
		start = segment.start / value,
		end = segment.end / value
	};
}
	
[Serializable]
public class Circle {
	public float radius;
	public Vec2 position;
	
	public static Circle operator /(Circle circle, float value) => new Circle{
		radius = circle.radius / value,
		position = circle.position / value
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
}

[Serializable]
public class Configuration {
	public float radius;
	public float width;
    public float height;
	public float scale;
	public RailSegment[] segments;
	public Circle[] targets;
	public ArucoMarkers markers;
	public CameraIntrinsics camera;
	public Plane ballPlane;
	public WorldToModel worldToModel;
	public Table table;
	public Correction correctionWidth;
	public Correction correctionHeight;

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
            worldToRail = new Vec3 { x = config.table.worldToRail.x, y = config.table.worldToRail.y, z = config.table.worldToRail.z }
        },
        camera = new CameraIntrinsics {fx = config.camera.fx, fy = config.camera.fy,
                           cx = config.camera.cx, cy = config.camera.cy,
                           skew = config.camera.skew,
                           k1 = config.camera.k1, k2 = config.camera.k2, k3 = config.camera.k3,
                           p1 = config.camera.p1, p2 = config.camera.p2,
                           sx = config.camera.sx, sy = config.camera.sy
        },
		correctionWidth = new Correction{factor = config.correctionWidth.factor, translation = config.correctionWidth.translation},
		correctionHeight = new Correction{factor = config.correctionHeight.factor, translation = config.correctionHeight.translation}
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
	public string type = "";
}

[Serializable]
public class RootState {
	public BallState[] balls = new BallState[0];
}

[Serializable]
public class BallState {
	public string type;
	public string id;
    public Vec2 position;
}

public class BallObjectInformation : MonoBehaviour
{
    public string id;
	public string type;
	public bool selectable;
}