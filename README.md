# Laptop-offloaded-LIDAR-SLAM-robot
Autonomous indoor mapping &amp; navigation robot built with ROS2 — LiDAR SLAM (slam_toolbox) and Nav2, with on-robot sensing offloaded to a laptop over WiFi. Differentiator:object detection feeding the navigation costmap
Stack: ROS2 Jazzy using Ubuntu 24.04. using python for most nodes and C++ for the costmap plugin.

Week 1 — ROS2 fundamentals 
Day 1 — 20/06/2026: Orientation and sequencing
Decided to do the official Jazzy beginner tutorials first, turtlesim for the sandbox learning nodes/topics/services, then doing the SLAM/Nav2 tutorial series, then hardware. The Addison series assumes you already know what a node, topic, and transform are.

Decision logged: turtlesim before autonomy-stack tutorials
Day 2–3 — 21-22/06/2026: Beginner CLI tools
Worked through the CLI section in the turtlesim sandbox — running nodes, ros2 topic / ros2 service / ros2 node inspection, rqt.

Learned about QoS matching: hit the ros2 topic pub -w 2 flag. I learned that publisher and subscriber must match on topic + type + QoS. Ros2 topic info <topic> shows pub/sub counts. This is important for Phase 3
Learned: rqt_graph shows topics and not services, services are request/response and don't appear in the topic graph.

Day 4–5 — 24-25/06/2026: Client libraries (writing nodes)
Started to writing nodes. Publishers and subscribers, services and clients, in Python (rclpy). Read the C++ versions alongside for familiarity, since the costmap plugin later is C++.

C++ vs Python: For services request/response, the language choice barely matters but for publishers and subscribers it matters more due to Python Global Interpreter Lock and the serialization overhead on big/fast messages. So Python is the right default for my nodes C++ reserved for the costmap plugin where pluginlib requires it.

Day 6–7 — 27-28/06/2026: Custom interfaces + parameters
Built the more_interfaces package (custom msg/srv). This is the pattern the whole project's cross-language fusion depends on.

rosidl: understood that .msg files are just definitions and rosidl_default_generators generates Python and C++ code from them at build time. rosidl_default_runtime is needed to use it. package.xml uses buildtool_depend or exec_depend to express the same split.
Decision logged: my custom messages will go in a dedicated standalone interfaces package so the Python detector node and the C++ costmap plugin can depend on it cleanly with no circular dependency.

Bug — value-vs-pointer: used -> on a bare AddressBook value, my confusion came forgetting the UniquePtr tag which would have made -> correct. Diagnosed by comparing my code against the known-working example to isolate the one difference.

Params concept: the parameters node declares once in __init__, then reads/rewrites in the loop — it does not re-declare each tick, and it does not create a publisher.

Subjects of note:

When completing the colcon builds tutorial everything under src/ is built by default my output showed more packages than the tutorial because I'd accumulated several tutorials. --packages-select <pkg> fixes the issue.
Removing a source package without deleting build/, install/, log/ leaves files that can still be sourced which can lead to bugs. Using rm -rf build install log and rebuild clean.
Overlay sourcing: The last file sourced takes precedent. setup.bash replays the full recorded prefix chain; local_setup.bash adds only the current workspace. 
. = source: . install/setup.bash and source install/setup.bash are identical.

Decision logged: keep tutorial packages in the learning workspace; the real robot workspace stays clean from commit one.

Week 2 — tf2 (coordinate frames)
Day 8 — 30/06/2026: Why tf2, and the frame tree
Started tf2: i understand it as a timestamped tree of frames, each with one parent, queryable at any past instant. 

Key concept — the map/odom split: odom → base_link is smooth but drifts; map → base_link is accurate but snaps on SLAM corrections. Can't be both, so odometry owns odom → base_link and slam_toolbox publishes map → odom as a drift-absorbing correction. This is the tree I'll wire by hand in Phase 3.

tf2_echo and reading transforms
Ran tf2_echo on the turtle demo. The output is one translation and one rotation shown four ways (quaternion, RPY radians, RPY degrees, 4×4 matrix) — same fact in different costumes.

Direction convention: tf2_echo <target> <source> / lookup_transform(target, source). The target is the eyes you're looking through and the source is the thing you're locating.
Identity quaternion = [0,0,0,1], not all zeros — w = cos(θ/2) = 1 at zero rotation; components normalize to 1. RPY does go all-zeros for no rotation.
Matrix: top-left 2×2 is the standard 2D rotation block, right column is translation, bottom [0 0 0 1] is the homogeneous trick. Walking the frame tree = multiplying these matrices.

Static broadcaster
Wrote the static broadcaster. The broadcaster publishes an edge of the tree, it gives a directed parent→child transform on the /tf topic. The child as seen from parent.

Relevance: this is exactly how I'll publish base_link → laser on the real robot.
Bug — ModuleNotFoundError: ros2 run found the entry-point wrapper but the import failed. Learned the wrapper is generated from setup.py entry_points without checking the module exists. The cause was a misplaced source file that was in the main package directory instead of the src directory.

Day 9 — 1/07/2026: Dynamic broadcaster + parameters in practice
Wrote the dynamic broadcaster (subscribes to a turtle's pose, publishes world → turtleN).

Cleared up: get_parameter_value().string_value returns the whole string, not a number; declare_parameter('turtlename', 'turtle') sets a default/fallback, an external value overrides it entirely.
Bug — launch file not found in share: The tutorial allowed for .yaml, .xml and .py file extensions but the tutorial used the .xml version in the command they gave whereas i used the .py extension when creating the launch file.

Listener + the read/write pair
Wrote the listener. This is the consumer half, it uses lookup_transform to read the tree and acts on the answer.

Key points: the TransformListener silently subscribes to /tf and fills a Buffer in the background. lookup_transform is a query against the buffer. The only literal messages in the node is the outgoing Twist velocity command to /turtle2/cmd_vel which the turtlesim node. Push-to-topic / pull-from-buffer asymmetry is why tf2 exists.
Placing a camera detection on the costmap is lookup_transform('map', 'camera', stamp) — structurally identical to the turtle chase. This is the multi-sensor fusion spine.


launch concepts: launch files can include other launch files via IncludeLaunchDescription, and pass values down with launch_arguments={'target_frame': 'carrot1'}. 

tf2 time + docs gap
Worked through "Using time" and "Traveling in time" concepts — the temporal axis of the buffer.
Docs note: the Jazzy docs only publish the C++ pages for these two tutorials — the Python pages exist on EOL distros (Galactic/Foxy) but 404 on Jazzy. Ill read the C++ for the concept and translated the single changed lookup_transform call to rclpy myself.

Error: get_clock().now() with a timeout has 100% failure in Python, works in C++
The "Using time" tutorial has you use now() in C++ to see it fail with "extrapolation into the future", then add a timeout as the fix. In C++ the timeout fixes but in python it fails anyway due to differences with python and c++ architecture. thee best fix is a query at rclpy.time.Time() which fetches the latest transform.

Error: lookup_transform vs lookup_transform_full -
"Traveling in time" uses the 6-argument advanced lookup,in C++ this is an overload of the lookupTransform function. However rclpy doesn't overload so the advanced form is a separate method, lookup_transform_full. 

Day 10 — 2/07/2026: Debugging + Quaternion + using stamped datatypes

tf2 debugging tutorial - Went over errors i have already encountered 
Quaternion fundamentals - RPY is my interface, quaternions are the plumbing, tf2 does the algebra.
Using stamped dataTypes - Uses MessageFilter to translate frame point of view. This is the tutorial that most directly rehearses the Phase 4 camera→costmap fusion, although a python version exists it is used little in comparison to it's C++ counterpart, docs only go over the c++ version.

Day 11 — 3/07/2026 URDF: 
tf2 queries the frame tree, URDF declares the robot's frames from geometry, and robot_state_publisher publishes them to /tf — replacing hand-written broadcasters.

Reused my learning workspace instead of the tutorial's second_ros2_ws — one workspace holds many packages; the rule that matters is keeping learning packages out of the future robot workspace, not one-per-tutorial.

<link> defines a frame.
robot_state_publisher automated tf broadcaster
joint_state_publisher made current joint angles, GUI sliders now and wheel encoders on the real robot.

Joint places the child frame from the parent.
Visual <origin> places geometry within its own frame, puerly cosmetic.

Joint types:
type declares the relationship; fixed (constant, no axis), continuous (free rotation, needs <axis>), revolute (limited, needs <limit>), prismatic (slide). Movable types need <axis> + a position from /joint_states.
My robot needs wheels, continuous, LiDAR = fixed.


Preprocessor with variables, math and macros that expands plain URDF before any tool reads it.
xmlns:xacro="http://www.ros.org/wiki/xacro" is a namespace label, not a link. Must be that exact string — XML doesn't care, but the xacro tool matches it literally.
Need it for two near-identical wheels, shared constants, and math.

create_publisher(JointState, 'joint_states', qos). jointState contains, type = header + name[] + position/velocity/effort[]; topic joint_states is a required convention.


Day 12 - 4/07/2026 - launch:

Worked through substitutions and event handlers in the launch system. Both are consequences of the same build-time/execute-time boundary.
A substitution is a deferred value across that gap, don't compute it now, compute it at launch time. Initially conflated this with IncludeLaunchDescription.
An event handler is a deferred reaction across the same gap: Events occur at runtime you don't declare the events, you declare the handler. 
Matchers to remember: OnProcessStart, OnProcessExit, OnProcessIO, OnShutdown, OnExecutionComplete. 

Status at end of Week 2

Done: beginner CLI + client libraries, custom interfaces, parameters, full tf2 sequence (broadcaster/listener/adding-a-frame/time).
Deliberately deferred: advanced intermediate tutorials as theyre out-of-scope. i will do pluginlib tutorial just before the Nav2 costmap plugin.
URDF → launch files → then Phase 2 (slam_toolbox + Nav2 in Gazebo simulation on a simulated TurtleBot3).

