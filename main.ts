namespace droneIMU {
    let initialized = false;

    export function init(): void {
        initialized = true;
    }

    export function shimSanityCheck(): boolean {
        return initialized;
    }
}
