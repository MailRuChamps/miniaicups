extern crate serde;
extern crate serde_json;
#[macro_use]
extern crate serde_derive;

use std::io::{self, Read};

#[derive(Serialize, Deserialize)]
struct Mine {
    Id: String,
    X: f32,
    Y: f32,
    R: f32,
    M: f32,
    SX: f32,
    SY: f32,
}

#[derive(Serialize, Deserialize)]
struct Objects {
    X: f32,
    Y: f32,
    T: String,
    Id: Option<String>,
    M: Option<f32>,
    R: Option<f32>,
}

#[derive(Serialize, Deserialize)]
struct Data {
    Mine: Vec<Mine>,
    Objects: Vec<Objects>,
}

#[derive(Serialize, Deserialize)]
struct Config {}

struct Strategy {
    pub config: Config,
}

#[derive(Serialize, Deserialize)]
struct Response {
    X: f32,
    Y: f32,
    Debug: Option<String>,
}

impl Response {
    fn new(x: f32, y: f32, d: &str) -> Response {
        Response {
            X: x,
            Y: y,
            Debug: Some(d.to_string()),
        }
    }
}

impl Strategy {
    fn run(self) {
        loop {
            let mut buffer = String::new();
            io::stdin().read_line(&mut buffer).unwrap();
            let r = self.tick(serde_json::from_str(&buffer).unwrap());
            println!("{}", serde_json::to_string(&r).unwrap());
        }
    }

    fn find_food(r: &Data) -> Option<&Objects> {
        r.Objects.iter().find(|&o| o.T == "F")
    }

    fn tick(&self, data: Data) -> Response {
        if data.Mine.is_empty() {
            return Response::new(0., 0., "Died");
        }
        if let Some(ref obj) = Strategy::find_food(&data) {
            Response::new(obj.X, obj.Y, "Go to food")
        } else {
            Response::new(0., 0., "No food!")
        }
    }
}

fn main() {
    let mut buffer = String::new();
    io::stdin().read_line(&mut buffer).unwrap();
    Strategy { config: serde_json::from_str(&buffer).unwrap() }.run()
}
