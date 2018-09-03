extern crate serde;
#[macro_use]
extern crate serde_derive;
extern crate serde_json;

use std::io::{self, Write};

use model::*;
use strategy::*;

/// Application entry point
fn main() {
    // Create instance of a dummy strategy.
    let strategy = DummyStrategy::new();

    // Run main loop
    run(strategy);
}

/// Run the main loop
fn run<T: Strategy>(mut strategy: T) {
    loop {
        // Read line from stdin
        let mut json = String::new();
        io::stdin().read_line(&mut json).unwrap();

        // Decode JSON message and process it
        let msg = serde_json::from_str(&json).unwrap();

        // Process the message
        let response = handle_message(msg, &mut strategy);

        // Check whether there's a response
        if response.is_none() {
            continue;
        }

        // Encode response to JSON
        let mut output = serde_json::to_string(&response.unwrap()).unwrap();
        output.push_str("\n");

        // Write response JSON to stdout
        io::stdout().write_all(&output.into_bytes()).unwrap();
    }
}

/// Handle input message
fn handle_message<T: Strategy>(msg: Message, strategy: &mut T) -> Option<Response> {
    // Handle each message type, produce optional response
    match msg.msg_type {
        MessageType::NewMatch => {
            // Extract message parameters
            let (my_lives, enemy_lives, proto_map, proto_car) = (
                msg.params.my_lives.unwrap(),
                msg.params.enemy_lives.unwrap(),
                msg.params.proto_map.unwrap(),
                msg.params.proto_car.unwrap()
            );

            // Call strategy's initialization function
            strategy.begin_match(my_lives, enemy_lives, proto_map, proto_car);

            // This message shouldn't be answered
            None
        }

        MessageType::Tick => {
            // Extract message parameters
            let (my_car, enemy_car, deadline_pos) = (
                msg.params.my_car.unwrap(),
                msg.params.enemy_car.unwrap(),
                msg.params.deadline_position.unwrap()
            );

            // Call strategy's main function
            let cmd = strategy.on_tick(my_car, enemy_car, deadline_pos);

            // Pass strategy's returned command plus a debug message
            Some(Response {
                command: cmd,
                debug: Some("Drive ahead!!".to_string()),
            })
        }
    }
}

/// The model for strategy JSON i/o
mod model {
    /// X, Y coordinates
    pub type Point2D = [f64; 2];

    /// X, Y coordinates + rotation angle
    pub type Transform2D = [f64; 3];

    /// The input message
    #[derive(Deserialize, Debug)]
    pub struct Message {
        #[serde(rename = "type")]
        pub msg_type: MessageType,
        pub params: MessageParams,
    }

    /// Input message type enum: begin match or update
    #[derive(Deserialize, Debug)]
    pub enum MessageType {
        #[serde(rename = "new_match")]
        NewMatch,

        #[serde(rename = "tick")]
        Tick,
    }

    /// Message payload
    #[derive(Deserialize, Debug)]
    pub struct MessageParams {
        pub my_lives: Option<i32>,
        pub enemy_lives: Option<i32>,
        pub proto_map: Option<ProtoMap>,
        pub proto_car: Option<ProtoCar>,
        pub my_car: Option<Car>,
        pub enemy_car: Option<Car>,
        pub deadline_position: Option<f64>,
    }

    /// Map description: static geometry of the scene
    #[derive(Deserialize, Debug)]
    pub struct ProtoMap {
        pub external_id: i32,
        pub segments: Vec<Segment>,
    }

    /// 2D segment: start point, end point, line thickness
    #[derive(Deserialize, Debug)]
    pub struct Segment(
        pub Point2D, pub Point2D, pub f64,
    );

    /// Car geometry and physics
    #[derive(Deserialize, Debug)]
    pub struct ProtoCar {
        pub external_id: i32,
        pub car_body_poly: Vec<Point2D>,
        pub rear_wheel_radius: f64,
        pub front_wheel_radius: f64,
        pub button_poly: Vec<Point2D>,

        pub car_body_mass: f64,
        pub car_body_friction: f64,
        pub car_body_elasticity: f64,
        pub max_speed: f64,
        pub torque: f64,
        pub drive: u32, // Drive type: 1=FF, 2=FR, 3=AWD

        pub rear_wheel_mass: f64,
        pub rear_wheel_position: Point2D,
        pub rear_wheel_friction: f64,
        pub rear_wheel_elasticity: f64,
        pub rear_wheel_joint: Point2D,
        pub rear_wheel_damp_position: Point2D,
        pub rear_wheel_damp_length: f64,
        pub rear_wheel_damp_stiffness: f64,
        pub rear_wheel_damp_damping: f64,

        pub front_wheel_mass: f64,
        pub front_wheel_position: Point2D,
        pub front_wheel_friction: f64,
        pub front_wheel_elasticity: f64,
        pub front_wheel_joint: Point2D,
        pub front_wheel_damp_position: Point2D,
        pub front_wheel_damp_length: f64,
        pub front_wheel_damp_stiffness: f64,
        pub front_wheel_damp_damping: f64,

        pub squared_wheels: Option<bool>,
    }

    /// Car state
    #[derive(Deserialize, Debug)]
    pub struct Car(
        pub Point2D,     // Car position (x, y)
        pub f64,         // Car rotation angle
        pub i32,         // Left/Right factor (+1/-1)
        pub Transform2D, // Wheel 1 (rear) position
        pub Transform2D, // Wheel 2 (front) position
    );

    /// The response message
    #[derive(Serialize, Debug)]
    pub struct Response {
        pub command: Command,
        pub debug: Option<String>,
    }

    /// The response command enum: left/right/stop
    #[derive(Serialize, Debug)]
    pub enum Command {
        #[serde(rename = "left")]
        Left,

        #[serde(rename = "right")]
        Right,

        #[serde(rename = "stop")]
        Stop,
    }
}

/// The strategy module
mod strategy {
    use model::*;

    /// The strategy. Implement this trait and pass instance to global function `run()`.
    pub trait Strategy {
        /// Callback for match begin.
        fn begin_match(&mut self, my_lives: i32, enemy_lives: i32, proto_map: ProtoMap, proto_car: ProtoCar);

        /// Callback for each tick.
        ///
        /// Must return the command for the car.
        fn on_tick(&mut self, my_car: Car, enemy_car: Car, deadline_pos: f64) -> Command;
    }

    /// Dummy strategy that moves left and right randomly
    pub struct DummyStrategy {
        tick: u32
    }

    impl DummyStrategy {
        pub fn new() -> DummyStrategy {
            DummyStrategy {
                tick: 0
            }
        }
    }

    impl Strategy for DummyStrategy {
        fn begin_match(&mut self, _my_lives: i32, _enemy_lives: i32, _proto_map: ProtoMap, _proto_car: ProtoCar) {
            self.tick = 0;
        }

        fn on_tick(&mut self, _my_car: Car, _enemy_car: Car, _deadline_pos: f64) -> Command {
            self.tick += 1;

            let t = self.tick % 9;

            if t < 4 {
                Command::Left
            } else if t > 5 {
                Command::Right
            } else {
                Command::Stop
            }
        }
    }
}
