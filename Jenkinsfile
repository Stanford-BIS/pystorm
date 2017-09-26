pipeline {
    agent any

    stages {
        stage('Build') {
            steps {
                script {
                    def lowercase_tag = "${BUILD_TAG.toLowerCase()}"
                    sh "docker build --file docker/Dockerfile_compile_source -t ${lowercase_tag} ."
                }
            }
        }
        stage('Test') {
            steps {
                script {
                    def lowercase_tag = "${BUILD_TAG.toLowerCase()}"
                    sh "docker/run_docker.sh ${lowercase_tag}"
                }
            }
        }
    }
}
