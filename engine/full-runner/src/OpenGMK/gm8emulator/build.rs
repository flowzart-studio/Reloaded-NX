use std::{
    env,
    error::Error,
    fs::{self},
    path::Path,
};

fn main() -> Result<(), Box<dyn Error>> {
    let out = env::var("OUT_DIR")?;

    // that one giant macro in kernel.rs
    let aa_macro_path = &Path::new(&out).join("_apply_args.macro.rs");
    if !aa_macro_path.is_file() {
        let i_char = |i| char::from(b'a' + i);
        let mut aa_macro = String::with_capacity(16384);
        aa_macro += &"macro_rules! _apply_args {\n    ($args: expr,) => { Ok(()) };\n".to_string();
        for i in 1..=16 {
            aa_macro += "    ($args: expr, ";
            for j in 0..i {
                aa_macro += &format!("${}: ident ", i_char(j));
            }
            aa_macro.truncate(aa_macro.len() - 1);
            aa_macro += ") => {{\n        match $args {\n            [";
            for j in 0..i {
                aa_macro.push(i_char(j));
                aa_macro += ", ";
            }
            aa_macro.truncate(aa_macro.len() - 2);
            aa_macro += "] => Ok(( ";
            for j in 0..i {
                aa_macro += &format!("_arg_into!(${0}, {0})?, ", i_char(j));
            }
            aa_macro.truncate(aa_macro.len() - 2);
            aa_macro += " )),\n            _ => unsafe { std::hint::unreachable_unchecked() },\n        }\n    }};\n";
        }
        aa_macro += "}\n";
        fs::write(aa_macro_path, &aa_macro)?;
    }

    // Horizon candidate uses reviewed pre-generated MIT-header bindings.
    println!("cargo:rerun-if-changed=src/render/opengl/horizon_mit.rs");
    Ok(())
}
