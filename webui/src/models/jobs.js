export default async function getJobs (http) {
  console.log('getJobs')
  try {
    const response = await http.get('http://localhost:3000/list/jobs')
    return response.data.jobs
  } catch (e) {
    console.warn(e)
  }
  return null
}
