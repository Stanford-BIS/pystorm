pipeline {
    agent any

    stages {
        stage('Build') {
            steps {
                sh 'docker build --file docker/Dockerfile_JENKINS_CI .'
            }
        }
    }
}
