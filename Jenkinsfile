pipeline {
    agent none
    stages {
        stage('Build (Debian Bullseye)') {
            agent {
                docker {
                    image 'debian:bullseye'
                }
            }
            steps {
                sh './scripts/ci/run.sh'
            }
            post {
                success {
                    archiveArtifacts artifacts: 'out/*.deb', fingerprint: true
                }
            }
        }
        stage('Build (Debian Buster)') {
            agent {
                docker {
                    image 'debian:buster'
                }
            }
            steps {
                sh './scripts/ci/run.sh'
            }
            post {
                success {
                    archiveArtifacts artifacts: 'out/*.deb', fingerprint: true
                }
            }
        }
    }
}
