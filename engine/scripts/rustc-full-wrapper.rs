//! Native wrapper avoids cmd.exe's 8191-character limit for winapi feature flags.
fn main() {
    let real=std::env::var_os("RELOADED_REAL_RUSTC").expect("real rustc");
    let linker=std::env::var("RELOADED_RUSTC_LINKER").expect("linker");
    let imports=std::env::var("RELOADED_NATIVE_IMPORTS").expect("native imports");
    let args:Vec<_>=std::env::args_os().skip(1).collect();
    let cross=args.windows(2).any(|a|a[0]=="--target"&&a[1].to_string_lossy().contains("aarch64"));
    let mut command=std::process::Command::new(&real);command.args(&args);
    // Source locations are diagnostic metadata, never development-machine paths.
    if let Ok(root)=std::env::var("RELOADED_SOURCE_ROOT") {
        command.arg("--remap-path-prefix").arg(format!("{root}=reloaded-nx"));
        command.arg("--remap-path-prefix").arg(format!("{}=reloaded-nx",root.replace('\\',"/")));
    }
    if let Ok(root)=std::env::var("CARGO_TARGET_DIR") {
        command.arg("--remap-path-prefix").arg(format!("{root}=build-output"));
        command.arg("--remap-path-prefix").arg(format!("{}=build-output",root.replace('\\',"/")));
    }
    if let Some(root)=std::path::Path::new(&real).parent().and_then(|bin|bin.parent()) {
        command.arg("--remap-path-prefix").arg(format!("{}=rust-toolchain",root.display()));
        command.arg("--remap-path-prefix").arg(format!("{}=rust-toolchain",root.to_string_lossy().replace('\\',"/")));
    }
    if cross {command.args(["-Z","unstable-options","-C","relocation-model=pic","-C","target-cpu=cortex-a57"]);}
    else {command.args(["-C","linker-flavor=ld","-C",&format!("linker={linker}"),"-C","link-self-contained=yes","-L",&format!("native={imports}")]);}
    let status=command.status().expect("spawn rustc");
    std::process::exit(status.code().unwrap_or(1));
}
