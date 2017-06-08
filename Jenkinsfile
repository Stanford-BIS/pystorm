pipeline {
    agent any

    stages {
        stage('Build') {
            steps {
                script {
                    def lowercase_tag = "${BUILD_TAG.toLowerCase()}"
                    sh "docker build --file docker/Dockerfile_JENKINS_CI -t ${lowercase_tag} ."
                }
            }
        }
        stage('Test') {
            steps {
                script {
                    def lowercase_tag = "${BUILD_TAG.toLowerCase()}"
                    sh "docker run --rm -i ${lowercase_tag} test ARGS=\"-V\" "
                }
            }
        }
    }
}
