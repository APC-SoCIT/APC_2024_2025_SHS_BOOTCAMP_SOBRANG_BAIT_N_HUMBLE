import { deleteDoc, doc } from "https://www.gstatic.com/firebasejs/10.12.0/firebase-firestore.js";

snapshot.forEach(docSnap => {
  const user = docSnap.data();
  const userId = docSnap.id;

  usersDiv.innerHTML += `
    <div>
      <p><strong>${user.name}</strong> - ${user.email}</p>
      <button onclick="deleteUser('${userId}')">Delete</button>
    </div>
  `;
});

window.deleteUser = async (userId) => {
  if (confirm("Are you sure you want to delete this user?")) {
    try {
      await deleteDoc(doc(db, "users", userId));
      alert("User deleted from Firestore and Firebase Auth.");
      location.reload();
    } catch (error) {
      console.error("Error deleting user:", error);
      alert("Failed to delete user.");
    }
  }
};
