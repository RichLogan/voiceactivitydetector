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
        .package(url: "https://github.com/gfreezy/libfvad", .upToNextMajor(from: "0.1.0")),
        .package(url: "https://github.com/Quick/Quick", .upToNextMajor(from: "6.0.0")),
        .package(url: "https://github.com/Quick/Nimble", .upToNextMajor(from: "12.0.0")),
    ],
    targets: [
        .target(
            name: "VoiceActivityDetector",
            dependencies: [
                .product(name: "libfvad", package: "libfvad"),
            ]
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
