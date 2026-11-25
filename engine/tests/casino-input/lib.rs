//! Contracts use the production Input, not a separate implementation.
#[path = "../../full-runner/src/OpenGMK/gm8emulator/src/types.rs"]
mod types;
#[path = "../../full-runner/src/OpenGMK/gm8emulator/src/input.rs"]
mod input;
#[path = "../../full-runner/src/OpenGMK/gm8emulator/src/casino_trace.rs"]
mod casino_trace;

#[cfg(test)]
mod regression {
    use super::input::Input;
    #[test]
    fn held_pressed_released_are_distinct_across_steps() {
        let mut i=Input::new();
        i.button_press(b'C',true);
        assert!(i.keyboard_check(b'C') && i.keyboard_check_pressed(b'C'));
        i.mouse_step();
        assert!(i.keyboard_check_pressed(b'C'));
        i.step();
        assert!(i.keyboard_check(b'C') && !i.keyboard_check_pressed(b'C'));
        i.button_release(b'C',true);
        assert!(!i.keyboard_check(b'C') && i.keyboard_check_released(b'C'));
        i.step();
        assert!(!i.keyboard_check_released(b'C'));
    }
    #[test]
    fn casino_cleanup_does_not_prevent_a_fresh_press() {
        let mut i=Input::new();
        for key in [32,b'Z',b'X',b'C'] {
            i.button_press(key,true);
            i.keyboard_key_release(key as i32);
            i.keyboard_clear(key);
            i.apply_pending_input();
            assert!(!i.keyboard_check(key));
            i.button_release(key,true);
            i.step();
            i.button_press(key,true);
            assert!(i.keyboard_check(key) && i.keyboard_check_pressed(key));
            i.button_release(key,true);
        }
    }
    #[test]
    fn modal_wait_receives_a_new_keyboard_lastkey() {
        let mut i=Input::new();
        i.set_keyboard_lastkey(0);
        i.button_press(b'Z',true);
        assert_eq!(i.keyboard_lastkey(),b'Z');
        i.button_release(b'Z',true);
        assert!(!i.keyboard_check(b'Z'));
    }
}
