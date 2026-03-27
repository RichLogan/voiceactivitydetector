// swift-tools-version:5.3
import PackageDescription

let package = Package(
    name: "VoiceActivityDetector",
    platforms: [
        .iOS(.v12),
    ],
    products: [
        .library(
            name: "VoiceActivityDetector",
            targets: ["VoiceActivityDetector"]
        ),
    ],
    dependencies: [
        .package(url: "https://github.com/Quick/Quick", .upToNextMajor(from: "6.0.0")),
        .package(url: "https://github.com/Quick/Nimble", .upToNextMajor(from: "12.0.0")),
    ],
    targets: [
        .target(
            name: "libfvad",
            path: "Sources/libfvad",
            sources: ["src"],
            publicHeadersPath: "include",
            cSettings: [
                .headerSearchPath("src"),
                .headerSearchPath("src/signal_processing"),
                .headerSearchPath("src/vad"),
            ]
        ),
        .target(
            name: "VoiceActivityDetector",
            dependencies: ["libfvad"]
        ),
        .testTarget(
            name: "VoiceActivityDetectorTests",
            dependencies: [
                "VoiceActivityDetector",
                .product(name: "Quick", package: "Quick"),
                .product(name: "Nimble", package: "Nimble"),
            ],
            resources: [
                .process("Resources"),
            ]
        ),
    ]
)
