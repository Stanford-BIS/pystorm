pipeline {
    agent any

    stages {
        stage('Build') {
            steps {
                script {
                    def lowercase_tag_release = "${BUILD_TAG.toLowerCase()}_release"
                    def lowercase_tag_debug = "${BUILD_TAG.toLowerCase()}_debug"
                    sh "docker build --file docker/Dockerfile_compile_source -t ${lowercase_tag_release} ."
                    sh "docker build --file docker/Dockerfile_compile_source -t ${lowercase_tag_debug} ."
                }
            }
        }
        stage('Test') {
            steps {
                script {
                    def lowercase_tag_release = "${BUILD_TAG.toLowerCase()}_release"
                    def lowercase_tag_debug = "${BUILD_TAG.toLowerCase()}_debug"
                    sh "docker/run_docker.sh ${lowercase_tag_release} Release"
                    sh "docker/run_docker.sh ${lowercase_tag_debug} Debug"
                }
            }
        }
    }
}
